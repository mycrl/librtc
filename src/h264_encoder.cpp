//
//  h264_encoder.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/3/10.
//

#include "h264_encoder.h"
#include "libyuv.h"
#include "frame.h"

extern "C"
{
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
}

std::vector<webrtc::SdpVideoFormat> H264Encoder::GetSupportedFormats()
{
	return supported_h264_codecs(true);
}

H264Encoder::H264Encoder(const webrtc::SdpVideoFormat& format)
{
	auto mode = format.parameters.find(cricket::kH264FmtpPacketizationMode);
	_codec_specific.codecSpecific.H264.packetization_mode =
		mode != format.parameters.end() && mode->second == "1"
		? webrtc::H264PacketizationMode::NonInterleaved
		: webrtc::H264PacketizationMode::SingleNalUnit;
	_codec_specific.codecType = webrtc::kVideoCodecH264;
}

std::unique_ptr<H264Encoder> H264Encoder::Create(const webrtc::SdpVideoFormat& format)
{
	return std::make_unique<H264Encoder>(format);
}

int32_t H264Encoder::RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback)
{
	_callback = callback;
	return CodecRet::Ok;
}

int H264Encoder::InitEncode(const webrtc::VideoCodec* codec_settings,
							const Settings& settings)
{
	int ret = 0;
	int number_of_streams = codec_settings->numberOfSimulcastStreams;
	if (number_of_streams > 1)
	{
		return CodecRet::Err;
	}

	auto codec_name = find_codec(VideoEncoders);
	if (codec_name == NULL)
	{
		return CodecRet::Err;
	}

	_codec = avcodec_find_encoder_by_name(codec_name);
	if (!_codec)
	{
		return CodecRet::Err;
	}

	_ctx = avcodec_alloc_context3(_codec);
	if (_ctx == NULL)
	{
		return CodecRet::Err;
	}

	_ctx->max_b_frames = 0;
	_ctx->width = codec_settings->width;
	_ctx->height = codec_settings->height;
	_ctx->bit_rate = codec_settings->maxBitrate * 1000;
	_ctx->framerate = av_make_q(codec_settings->maxFramerate, 1);
	_ctx->time_base = av_make_q(1, codec_settings->maxFramerate);
	_ctx->pkt_timebase = av_make_q(1, codec_settings->maxFramerate);
	_ctx->gop_size = codec_settings->H264().keyFrameInterval;

	if (codec_name == "h264_qsv")
	{
		_ctx->pix_fmt = AV_PIX_FMT_NV12;
	}
	else
	{
		_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	}

	if (codec_name == "h264_videotoolbox")
	{
		av_opt_set_int(_ctx->priv_data, "prio_speed", 1, 0);
		av_opt_set_int(_ctx->priv_data, "realtime", 1, 0);
	}
	else if (codec_name == "h264_qsv")
	{
		av_opt_set_int(_ctx->priv_data, "preset", 7, 0);
		av_opt_set_int(_ctx->priv_data, "profile", 66, 0);
	}
	else if (codec_name == "h264_nvenc")
	{
		av_opt_set_int(_ctx->priv_data, "zerolatency", 1, 0);
		av_opt_set_int(_ctx->priv_data, "b_adapt", 0, 0);
		av_opt_set_int(_ctx->priv_data, "rc", 1, 0);
		av_opt_set_int(_ctx->priv_data, "preset", 3, 0);
		av_opt_set_int(_ctx->priv_data, "profile", 0, 0);
		av_opt_set_int(_ctx->priv_data, "tune", 1, 0);
		av_opt_set_int(_ctx->priv_data, "cq", 30, 0);
	}
	else if (codec_name == "libx264")
	{
		av_opt_set(_ctx->priv_data, "tune", "zerolatency", 0);
	}

	if (avcodec_open2(_ctx, _codec, NULL) != 0)
	{
		return CodecRet::Err;
	}

	if (avcodec_is_open(_ctx) == 0)
	{
		return CodecRet::Err;
	}

	_packet = av_packet_alloc();
	if (_packet == NULL)
	{
		return CodecRet::Err;
	}

	_frame = av_frame_alloc();
	if (_frame == NULL)
	{
		return -1;
	}

	_frame_num = 0;
	_frame->width = _ctx->width;
	_frame->height = _ctx->height;
	_frame->format = _ctx->pix_fmt;

	if (_codec->name == "h264_qsv" || _codec->name == "libx264")
	{
		ret = av_image_alloc(_frame->data,
							 _frame->linesize,
							 _ctx->width,
							 _ctx->height,
							 _ctx->pix_fmt,
							 32);
	}
	else if (_codec->name == "libx264")
	{
		ret = av_frame_get_buffer(_frame, 32);
	}

	if (ret < 0)
	{
		return -1;
	}

	return CodecRet::Ok;
}

int32_t H264Encoder::Encode(const webrtc::VideoFrame& frame,
							const std::vector<webrtc::VideoFrameType>* frame_types)
{
	if (!_callback || !_ctx)
	{
		return CodecRet::Err;
	}

	if (!frame_types)
	{
		return CodecRet::Ok;
	}

	auto video_frame_buffer = frame.video_frame_buffer();
	auto i420_buffer = video_frame_buffer->GetI420();
	if (!i420_buffer)
	{
		return CodecRet::Err;
	}

	if (_codec->name == "h264_qsv")
	{
		int ret = libyuv::I420ToNV12(i420_buffer->DataY(),
									 i420_buffer->StrideY(),
									 i420_buffer->DataU(),
									 i420_buffer->StrideU(),
									 i420_buffer->DataV(),
									 i420_buffer->StrideV(),
									 _frame->data[0],
									 _frame->linesize[0],
									 _frame->data[1],
									 _frame->linesize[1],
									 i420_buffer->width(),
									 i420_buffer->height());
		if (ret != 0)
		{
			return CodecRet::Err;
		}
	}
	else if (_codec->name == "libx264")
	{
		int ret = av_frame_make_writable(_frame);
		if (ret != 0)
		{
			return CodecRet::Ok;
		}

		int need_size = av_image_fill_arrays(_frame->data,
											 _frame->linesize,
											 i420_buffer->DataY(),
											 _ctx->pix_fmt,
											 i420_buffer->width(),
											 i420_buffer->height(),
											 1);
		size_t size = get_i420_buffer_size(i420_buffer);
		if (need_size != size)
		{
			return CodecRet::Ok;
		}
	}
	else
	{
		_frame->linesize[0] = (int)i420_buffer->StrideY();
		_frame->linesize[1] = (int)i420_buffer->StrideU();
		_frame->linesize[2] = (int)i420_buffer->StrideV();

		_frame->data[0] = (uint8_t*)i420_buffer->DataY();
		_frame->data[1] = (uint8_t*)i420_buffer->DataU();
		_frame->data[2] = (uint8_t*)i420_buffer->DataV();
	}

	for (auto frame_type : *frame_types)
	{
		if (frame_type == webrtc::VideoFrameType::kEmptyFrame)
		{
			continue;
		}

		if ((_OnFrame(frame_type)) != 0)
		{
			break;
		}

		while (true)
		{
			if (_ReadPacket(frame_type, frame) != 0)
			{
				break;
			}
		}
	}

	return CodecRet::Ok;
}

void H264Encoder::SetRates(const webrtc::VideoEncoder::RateControlParameters& parameters)
{
	_ctx->bit_rate = parameters.bitrate.get_sum_bps();
	_ctx->framerate = av_make_q(parameters.framerate_fps, 1);
	_ctx->time_base = av_make_q(1, parameters.framerate_fps);
	_ctx->pkt_timebase = av_make_q(1, parameters.framerate_fps);
}

H264Encoder::EncoderInfo H264Encoder::GetEncoderInfo() const
{
	EncoderInfo info;
	info.requested_resolution_alignment = 4;
	info.supports_native_handle = false;
	info.implementation_name = std::string(_codec->name);
	info.has_trusted_rate_controller = false;
	info.is_hardware_accelerated = true;
	info.supports_simulcast = false;
	return info;
}

int32_t H264Encoder::Release()
{
	if (_codec)
	{
		avcodec_send_frame(_ctx, NULL);
		avcodec_free_context(&_ctx);
		av_packet_free(&_packet);
		av_frame_free(&_frame);
	}

	return CodecRet::Ok;
}

int H264Encoder::_ReadPacket(webrtc::VideoFrameType frame_type,
							 const webrtc::VideoFrame& frame)
{
	int ret = avcodec_receive_packet(_ctx, _packet);
	if (ret != 0)
	{
		return -1;
	}

	auto img_buf = webrtc::EncodedImageBuffer::Create(_packet->data, _packet->size);
	auto qp = _h264_bitstream_parser.GetLastSliceQp();
	if (!qp.has_value())
	{
		auto buf = rtc::ArrayView<const uint8_t>(_packet->data, _packet->size);
		_h264_bitstream_parser.ParseBitstream(buf);
	}

	_image.SetSpatialIndex(0);
	_image.SetEncodedData(img_buf);
	_image.set_size(_packet->size);
	_image._frameType = frame_type;
	_image._encodedWidth = _ctx->width;
	_image._encodedHeight = _ctx->height;
	_image.SetTimestamp(frame.timestamp());
	_image.ntp_time_ms_ = frame.ntp_time_ms();
	_image.capture_time_ms_ = frame.render_time_ms();
	_image.rotation_ = frame.rotation();
	_image.qp_ = qp.value_or(-1);

	_callback->OnEncodedImage(_image, &_codec_specific);
	av_packet_unref(_packet);
	return 0;
}

int H264Encoder::_OnFrame(webrtc::VideoFrameType frame_type)
{
	if (frame_type == webrtc::VideoFrameType::kVideoFrameKey)
	{
		_frame->key_frame = 1;
	}
	else
	{
		_frame->key_frame = 0;
	}

	_frame->pts = av_rescale_q(_frame_num,
							   _ctx->pkt_timebase,
							   _ctx->time_base);
	if (avcodec_send_frame(_ctx, _frame) != 0)
	{
		return -1;
	}
	else
	{
		_frame_num++;
	}

	return 0;
}
