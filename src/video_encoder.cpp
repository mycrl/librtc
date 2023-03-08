//
//  video_encoder.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "video_encoder.h"

/* Video Encoder */

Encoder::Encoder()
{
    codec = avcodec_find_encoder_by_name("libx264");
}

void Encoder::Release()
{
    avcodec_send_frame(ctx, NULL);
    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&ctx);
}

std::unique_ptr<IVideoEncoder> IVideoEncoder::Create(const webrtc::SdpVideoFormat& format)
{
    return std::make_unique<IVideoEncoder>(format);
}

int IVideoEncoder::InitEncode(const webrtc::VideoCodec* codec_settings, const Settings& settings)
{
    _codec_settings = codec_settings;
    int number_of_streams = codec_settings->numberOfSimulcastStreams;
    
    for (int i = 0, idx = number_of_streams - 1; i < number_of_streams; ++i, --idx)
    {
        auto encoder = open_encoder(&codec_settings->simulcastStream[i], idx);
        if (encoder.has_value())
        {
            _encoders.push_back(encoder.value());
        }
    }
    
    return WEBRTC_VIDEO_CODEC_OK;
}

int32_t IVideoEncoder::RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback)
{
    _callback = callback;
    return WEBRTC_VIDEO_CODEC_OK;
}

int32_t IVideoEncoder::Encode(const webrtc::VideoFrame& frame,
                              const std::vector<webrtc::VideoFrameType>* frame_types)
{
    if (!_callback)
    {
        return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
    }
    
    if (!frame_types)
    {
        return WEBRTC_VIDEO_CODEC_OK;
    }
    
    int ret;
    auto buf = frame.video_frame_buffer()->GetI420();
    for (auto frame_type: *frame_types)
    {
        if (frame_type == webrtc::VideoFrameType::kEmptyFrame)
        {
            continue;
        }
        
        for (auto encoder: _encoders)
        {
            ret = av_frame_make_writable(encoder.frame);
            if (ret != 0)
            {
                continue;
            }
            
            encoder.frame->pts += 1;
            ret = avcodec_send_frame(encoder.ctx, encoder.frame);
            if(ret != 0)
            {
                return WEBRTC_VIDEO_CODEC_ERROR;
            }
            
            for (;;)
            {
                ret = avcodec_receive_packet(encoder.ctx, encoder.packet);
                if (ret != 0)
                {
                    break;
                }
                
                _h264_bitstream_parser.ParseBitstream(rtc::ArrayView<const uint8_t>(encoder.packet->data, encoder.packet->size));
                
                encoder.image.SetEncodedData(webrtc::EncodedImageBuffer::Create(encoder.packet->data, encoder.packet->size));
                encoder.image.SetSpatialIndex(encoder.simulcast_idx);
                encoder.image.SetTimestamp(frame.timestamp());
                encoder.image._encodedWidth = encoder.ctx->width;
                encoder.image._encodedHeight = encoder.ctx->height;
                encoder.image.set_size(encoder.packet->size);
                encoder.image._frameType = frame_type;
                encoder.image.ntp_time_ms_ = frame.ntp_time_ms();
                encoder.image.capture_time_ms_ = frame.render_time_ms();
                encoder.image.rotation_ = frame.rotation();
                encoder.image.qp_ = _h264_bitstream_parser.GetLastSliceQp().value_or(-1);
                
                webrtc::CodecSpecificInfo codec_specific;
                codec_specific.codecType = webrtc::kVideoCodecH264;
                codec_specific.codecSpecific.H264.packetization_mode = encoder.packetization_mode;
                
                _callback->OnEncodedImage(encoder.image, &codec_specific);
            }
        }
    }
    
    return WEBRTC_VIDEO_CODEC_OK;
}

void IVideoEncoder::SetRates(const webrtc::VideoEncoder::RateControlParameters& parameters)
{
    for (auto encoder: _encoders)
    {
        encoder.ctx->bit_rate = parameters.bitrate.get_sum_bps() / 1000;
        encoder.ctx->framerate = av_make_q(parameters.framerate_fps, 1);
        encoder.ctx->time_base = av_make_q(1, parameters.framerate_fps);
        encoder.ctx->pkt_timebase = av_make_q(1, parameters.framerate_fps);
    }
}

int32_t IVideoEncoder::Release()
{
    for (auto encoder: _encoders)
    {
        encoder.Release();
    }
    
    return WEBRTC_VIDEO_CODEC_OK;
}

std::optional<Encoder> IVideoEncoder::open_encoder(const webrtc::SimulcastStream* stream,
                                                   int stream_idx)
{
    int ret;
    
    Encoder encoder;
    encoder.simulcast_idx = stream_idx;
    
    auto pars = _format.parameters;
    auto mode = pars.find(cricket::kH264FmtpPacketizationMode);
    if (mode != pars.end() && mode->second == "1")
    {
        encoder.packetization_mode = webrtc::H264PacketizationMode::NonInterleaved;
    }
    else
    {
        encoder.packetization_mode = webrtc::H264PacketizationMode::SingleNalUnit;
    }
    
    encoder.ctx = avcodec_alloc_context3(encoder.codec);
    if (encoder.ctx == NULL)
    {
        return std::nullopt;
    }
    
    if (encoder.codec->id != AV_CODEC_ID_H264)
    {
        return std::nullopt;
    }
    
    encoder.ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    encoder.ctx->width = stream->width;
    encoder.ctx->height = stream->height;
    encoder.ctx->framerate = av_make_q(stream->maxFramerate, 1);
    encoder.ctx->time_base = av_make_q(1, stream->maxFramerate);
    encoder.ctx->pkt_timebase = av_make_q(1, stream->maxFramerate);
    
    encoder.ctx->max_b_frames = 3;
    encoder.ctx->bit_rate = stream->targetBitrate * 1000 * 0.7;
    encoder.ctx->gop_size = _codec_settings->H264().keyFrameInterval;
    
    ret = avcodec_open2(encoder.ctx, encoder.codec, NULL);
    if (ret != 0)
    {
        return std::nullopt;
    }
    
    encoder.packet = av_packet_alloc();
    if (encoder.packet == NULL)
    {
        return std::nullopt;
    }
    
    encoder.frame = av_frame_alloc();
    if (encoder.frame == NULL)
    {
        return std::nullopt;
    }
    
    encoder.frame->format = encoder.ctx->pix_fmt;
    encoder.frame->width = encoder.ctx->width;
    encoder.frame->height = encoder.ctx->height;
    
    ret = av_frame_get_buffer(encoder.frame, 32);
    if (ret < 0)
    {
        return std::nullopt;
    }
    
    return std::move(encoder);
}

/* Video Encoder Factory */

std::unique_ptr<IVideoEncoderFactory> IVideoEncoderFactory::Create()
{
    return std::make_unique<IVideoEncoderFactory>();
}

std::vector<webrtc::SdpVideoFormat> IVideoEncoderFactory::GetSupportedFormats() const
{
    return (*_factory)->GetSupportedFormats();
}

std::unique_ptr<webrtc::VideoEncoder> IVideoEncoderFactory::CreateVideoEncoder(const webrtc::SdpVideoFormat& format)
{
    if (format.name == "H264")
    {
        return IVideoEncoder::Create(format);;
    }
    
    if (!_factory)
    {
        _factory = webrtc::CreateBuiltinVideoEncoderFactory();
    }
    
    return (*_factory)->CreateVideoEncoder(format);
}
