//
//  h264_encoder.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/3/10.
//

#include "h264_encoder.h"
#include "libyuv.h"
#include "h264.h"

extern "C"
{
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
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

std::vector<webrtc::SdpVideoFormat> H264Encoder::GetSupportedFormats()
{
    return ISupportedH264Codecs(true);
}

std::unique_ptr<H264Encoder> H264Encoder::Create(const webrtc::SdpVideoFormat& format)
{
    return std::make_unique<H264Encoder>(format);
}

int32_t H264Encoder::RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback)
{
    _callback = callback;
    return WEBRTC_VIDEO_CODEC_OK;
}

int H264Encoder::InitEncode(const webrtc::VideoCodec* codec_settings, const Settings& settings)
{
    int ret;
    int number_of_streams = codec_settings->numberOfSimulcastStreams;
    if (number_of_streams > 1)
    {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    
    for (auto name: Encoders)
    {
        _codec = avcodec_find_encoder_by_name(name.c_str());
        if (_codec)
        {
            _name = name;
            break;
        }
    }
    
    if (!_codec)
    {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    
    _ctx = avcodec_alloc_context3(_codec);
    if (_ctx == NULL)
    {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    
    if (_name == "h264_qsv")
    {
        _ctx->pix_fmt = AV_PIX_FMT_NV12;
    }
    else
    {
        _ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    }
    
    _ctx->max_b_frames = 0;
    _ctx->width = codec_settings->width;
    _ctx->height = codec_settings->height;
    _ctx->bit_rate = codec_settings->maxBitrate * 1000;
    _ctx->framerate = av_make_q(codec_settings->maxFramerate, 1);
    _ctx->time_base = av_make_q(1, codec_settings->maxFramerate);
    _ctx->pkt_timebase = av_make_q(1, codec_settings->maxFramerate);
    _ctx->gop_size = codec_settings->H264().keyFrameInterval;
    
    if (_name == "h264_videotoolbox")
    {
        av_opt_set_int(_ctx->priv_data, "prio_speed", 1, 0);
        av_opt_set_int(_ctx->priv_data, "realtime", 1, 0);
    }
    
    if (_name == "h264_nvenc")
    {
        av_opt_set_int(_ctx->priv_data, "zerolatency", 1, 0);
        av_opt_set_int(_ctx->priv_data, "b_adapt", 0, 0);
        av_opt_set_int(_ctx->priv_data, "rc", 1, 0);
        av_opt_set_int(_ctx->priv_data, "preset", 3, 0);
        av_opt_set_int(_ctx->priv_data, "profile", 0, 0);
        av_opt_set_int(_ctx->priv_data, "tune", 1, 0);
        av_opt_set_int(_ctx->priv_data, "cq", 30, 0);
        av_opt_set_int(_ctx->priv_data, "multipass", 2, 0);
    }
    
    if (_name == "libx264")
    {
        av_opt_set(_ctx->priv_data, "tune", "zerolatency", 0);
    }
    
    ret = avcodec_open2(_ctx, _codec, NULL);
    if (ret != 0)
    {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    
    _packet = av_packet_alloc();
    if (_packet == NULL)
    {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    
    _frame = av_frame_alloc();
    if (_frame == NULL)
    {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    
    _frame->width = _ctx->width;
    _frame->height = _ctx->height;
    _frame->format = _ctx->pix_fmt;
    ret = av_frame_get_buffer(_frame, 32);
    if (ret < 0)
    {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    
    return WEBRTC_VIDEO_CODEC_OK;
}

int32_t H264Encoder::Encode(const webrtc::VideoFrame& frame,
                            const std::vector<webrtc::VideoFrameType>* frame_types)
{
    if (!_callback)
    {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    
    if (!frame_types)
    {
        return WEBRTC_VIDEO_CODEC_OK;
    }
    
    auto vfb = frame.video_frame_buffer();
    auto vb = vfb->GetI420();
    if (!vb)
    {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    
    int width = vb->width();
    int height = vb->height();
    int stride_y = vb->StrideY();
    int stride_u = vb->StrideU();
    int size_y = stride_y * height;
    int size_uv = stride_u * (height / 2);
    int len = size_y + (size_uv * 2);
    
    const uint8_t* buf = _ctx->pix_fmt == AV_PIX_FMT_NV12
        ? (uint8_t*)malloc(sizeof(uint8_t) * len)
        : vb->DataY();
    
    if (_ctx->pix_fmt == AV_PIX_FMT_NV12)
    {
        libyuv::I420ToNV12(vb->DataY(),
                           stride_y,
                           vb->DataU(),
                           stride_u,
                           vb->DataV(),
                           vb->StrideV(),
                           (uint8_t*)buf,
                           stride_y,
                           (uint8_t*)buf + size_y,
                           stride_u * 2,
                           width,
                           height);
    }

    for (auto frame_type: *frame_types)
    {
        if (frame_type == webrtc::VideoFrameType::kEmptyFrame)
        {
            continue;
        }
        
        if ((_OnFrame(frame_type,
                      buf,
                      width,
                      height,
                      len)) != 0)
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
    
    if (_ctx->pix_fmt == AV_PIX_FMT_NV12)
    {
        free((void*)buf);
    }
    
    return WEBRTC_VIDEO_CODEC_OK;
}

void H264Encoder::SetRates(const webrtc::VideoEncoder::RateControlParameters& parameters)
{
    _ctx->bit_rate = parameters.bitrate.get_sum_bps();
    _ctx->framerate = av_make_q(parameters.framerate_fps, 1);
    _ctx->time_base = av_make_q(1, parameters.framerate_fps);
    _ctx->pkt_timebase = av_make_q(1, parameters.framerate_fps);
}

int32_t H264Encoder::Release()
{
    avcodec_send_frame(_ctx, NULL);
    av_frame_free(&_frame);
    av_packet_free(&_packet);
    avcodec_free_context(&_ctx);
    return WEBRTC_VIDEO_CODEC_OK;
}

int H264Encoder::_ReadPacket(webrtc::VideoFrameType frame_type,
                             const webrtc::VideoFrame& frame)
{
    int ret = avcodec_receive_packet(_ctx, _packet);
    if (ret != 0)
    {
        return -1;
    }
    
    auto img_buf = webrtc::EncodedImageBuffer::Create(_packet->data,_packet->size);
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

int H264Encoder::_OnFrame(webrtc::VideoFrameType frame_type,
                          const uint8_t* frame_buf,
                          int width,
                          int height,
                          size_t len)
{
    int ret;
    
    if (frame_type == webrtc::VideoFrameType::kVideoFrameKey)
    {
        _frame->pict_type = AV_PICTURE_TYPE_I;
    }
    else
    {
        _frame->pict_type = AV_PICTURE_TYPE_NONE;
    }
    
    ret = av_frame_make_writable(_frame);
    if (ret != 0)
    {
        return 0;
    }
    
    int need_size = av_image_fill_arrays(_frame->data,
                                         _frame->linesize,
                                         frame_buf,
                                         (enum AVPixelFormat)_frame->format,
                                         width,
                                         height,
                                         1);
    if (need_size != len)
    {
        return -1;
    }
    
    ret = avcodec_send_frame(_ctx, _frame);
    if(ret != 0)
    {
        return -1;
    }
    
    _frame->pts ++;
    _frame->pkt_dts ++;
    return 0;
}
