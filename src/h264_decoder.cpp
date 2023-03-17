//
//  h264_decoder.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/3/16.
//

#include "api/video/i420_buffer.h"
#include "h264_decoder.h"

std::vector<webrtc::SdpVideoFormat> H264Decoder::GetSupportedFormats()
{
    return ISupportedH264Codecs(true);
}

std::unique_ptr<H264Decoder> H264Decoder::Create()
{
    return std::make_unique<H264Decoder>();
}

bool H264Decoder::Configure(const Settings& settings)
{
    _layer = find_decoder();
    if (!_layer.codec)
    {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    
    _ctx = avcodec_alloc_context3(_layer.codec);
    if (_ctx == NULL)
    {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    
    if (avcodec_open2(_ctx, _layer.codec, NULL) != 0)
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
    
    return WEBRTC_VIDEO_CODEC_OK;
}

int32_t H264Decoder::Decode(const webrtc::EncodedImage& input_image,
                            bool _missing_frames,
                            int64_t render_time_ms)
{
    if (!_layer.codec)
    {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    
    _packet->size = (int)input_image.size();
    _packet->data = (uint8_t*)input_image.data();
    if (avcodec_send_packet(_ctx, _packet) != 0)
    {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    
    while (true)
    {
        if (_OnFrame() == -1)
        {
            break;
        }
    }
    
    return WEBRTC_VIDEO_CODEC_OK;
}

int32_t H264Decoder::RegisterDecodeCompleteCallback(webrtc::DecodedImageCallback* callback)
{
    _callback = callback;
    return WEBRTC_VIDEO_CODEC_OK;
}

int32_t H264Decoder::Release()
{
    avcodec_send_frame(_ctx, NULL);
    av_frame_free(&_frame);
    av_packet_free(&_packet);
    avcodec_free_context(&_ctx);
    return WEBRTC_VIDEO_CODEC_OK;
}

int H264Decoder::_OnFrame()
{
    if (avcodec_receive_frame(_ctx, _frame) != 0)
    {
        return -1;
    }
    
    if (_frame->format != AV_PIX_FMT_YUV420P)
    {
        return 0;
    }
    
    int64_t time_ms =_frame->pts * (_frame->time_base.num * 1000 / _frame->time_base.den);
    webrtc::VideoFrame frame(webrtc::I420Buffer::Copy(_frame->width,
                                                      _frame->height,
                                                      _frame->data[0],
                                                      _frame->linesize[0],
                                                      _frame->data[1],
                                                      _frame->linesize[1],
                                                      _frame->data[2],
                                                      _frame->linesize[2]),
                             webrtc::kVideoRotation_0,
                             time_ms * rtc::kNumMicrosecsPerMillisec);
    _callback->Decoded(frame);
    return 0;
}
