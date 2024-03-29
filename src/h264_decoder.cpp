//
//  h264_decoder.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/3/16.
//

#include "h264_decoder.h"
#include "libyuv.h"
#include "frame.h"

H264Decoder::~H264Decoder()
{
	Release();
}

std::vector<webrtc::SdpVideoFormat> H264Decoder::GetSupportedFormats()
{
	return supported_h264_codecs(true);
}

std::unique_ptr<H264Decoder> H264Decoder::Create()
{
	return std::make_unique<H264Decoder>();
}

bool H264Decoder::Configure(const Settings& settings)
{
	auto codec_name = find_codec(VideoDecoders);
	_codec = avcodec_find_decoder_by_name(codec_name.c_str());
	if (!_codec)
	{
		return false;
	}

	_ctx = avcodec_alloc_context3(_codec);
	if (_ctx == nullptr)
	{
		return false;
	}

	if (avcodec_open2(_ctx, _codec, nullptr) != 0)
	{
		return false;
	}

	if (avcodec_is_open(_ctx) == 0)
	{
		return false;
	}

	_parser = av_parser_init(_codec->id);
	if (!_parser)
	{
		return false;
	}

	_packet = av_packet_alloc();
	if (_packet == nullptr)
	{
		return false;
	}

	_frame = av_frame_alloc();
	return _frame != nullptr;
}

int32_t H264Decoder::Decode(const webrtc::EncodedImage& input_image,
							bool _missing_frames,
							int64_t render_time_ms)
{
	if (!_callback || !_codec)
	{
		return CodecRet::Err;
	}

	uint8_t* data = (uint8_t*)input_image.data();
	int data_size = (int)input_image.size();

	while (data_size > 0)
	{
		int ret = av_parser_parse2(_parser,
								   _ctx,
								   &_packet->data,
								   &_packet->size,
								   data,
								   data_size,
								   AV_NOPTS_VALUE,
								   AV_NOPTS_VALUE,
								   0);
		if (ret < 0)
		{
			return CodecRet::Err;
		}

		data += ret;
		data_size -= ret;

		if (_packet->size == 0)
		{
			continue;
		}

		if (avcodec_send_packet(_ctx, _packet) != 0)
		{
			return CodecRet::Err;
		}

		while (true)
		{
			if (_ReadFrame(input_image, render_time_ms) != 0)
			{
				break;
			}
		}
	}

	return CodecRet::Ok;
}

int32_t H264Decoder::RegisterDecodeCompleteCallback(webrtc::DecodedImageCallback* callback)
{
	assert(callback);

	_callback = callback;
	return CodecRet::Ok;
}

int32_t H264Decoder::Release()
{
	if (_codec == nullptr)
	{
		return CodecRet::Ok;
	}
	else
	{
		_codec = nullptr;
	}

	avcodec_send_frame(_ctx, nullptr);
	avcodec_free_context(&_ctx);
	av_parser_close(_parser);
	av_packet_free(&_packet);
	av_frame_free(&_frame);
	
	return CodecRet::Ok;
}

int H264Decoder::_ReadFrame(const webrtc::EncodedImage& input_image,
							int64_t render_time_ms)
{
	if (avcodec_receive_frame(_ctx, _frame) != 0)
	{
		return -1;
	}

	if (!_i420_buffer.has_value())
	{
		bool is_nv12 = _frame->format == AV_PIX_FMT_NV12;
		int stride_u = is_nv12 ? _frame->linesize[1] / 2 : _frame->linesize[1];
		int stride_v = is_nv12 ? stride_u : _frame->linesize[2];
		_i420_buffer = webrtc::I420Buffer::Create(_frame->width,
												  _frame->height,
												  _frame->linesize[0],
												  stride_u,
												  stride_v);
	}

	if (_frame->format == AV_PIX_FMT_NV12)
	{
		libyuv::NV12ToI420(_frame->data[0],
						   _frame->linesize[0],
						   _frame->data[1],
						   _frame->linesize[1],
						   (uint8_t*)_i420_buffer.value()->DataY(),
						   _i420_buffer.value()->StrideY(),
						   (uint8_t*)_i420_buffer.value()->DataU(),
						   _i420_buffer.value()->StrideU(),
						   (uint8_t*)_i420_buffer.value()->DataV(),
						   _i420_buffer.value()->StrideV(),
						   _frame->width,
						   _frame->height);
	}
	else
	{
		_i420_buffer.value()->Copy(_frame->width,
						   _frame->height,
						   _frame->data[0],
						   _frame->linesize[0],
						   _frame->data[1],
						   _frame->linesize[1],
						   _frame->data[2],
						   _frame->linesize[2]);
	}

	webrtc::VideoFrame frame(_i420_buffer.value(),
							 webrtc::kVideoRotation_0,
							 render_time_ms * rtc::kNumMicrosecsPerMillisec);
	frame.set_timestamp(input_image.Timestamp());
	frame.set_ntp_time_ms(input_image.ntp_time_ms_);
	if (_callback.has_value())
	{
		_callback.value()->Decoded(frame);
	}

	return 0;
}