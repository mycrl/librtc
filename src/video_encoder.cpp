//
//  video_encoder.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "video_encoder.h"

/* Video Encoder */

std::unique_ptr<IVideoEncoder> IVideoEncoder::Create(VideoEncoderConfig* config)
{
    auto self = std::make_unique<IVideoEncoder>();
    self->config = config;
    return self;
}

int IVideoEncoder::InitEncode(const webrtc::VideoCodec* codec_settings, 
                              const Settings& settings)
{
    _codec = codec_settings;
    
    CodecSettings settings_;
    settings_.width = codec_settings->width;
    settings_.height = codec_settings->height;
    settings_.start_bitrate = codec_settings->startBitrate;
    settings_.max_bitrate = codec_settings->maxBitrate;
    settings_.min_bitrate = codec_settings->minBitrate;
    settings_.max_framerate = codec_settings->maxFramerate;
    settings_.active = codec_settings->active;
    settings_.qp_max = codec_settings->qpMax;
    settings_.number_of_simulcast_streams = codec_settings->numberOfSimulcastStreams;
    settings_.expect_encode_from_texture = codec_settings->expect_encode_from_texture;
    settings_.legacy_conference_mode = codec_settings->legacy_conference_mode;
    settings_.loss_notification = settings.capabilities.loss_notification;
    settings_.number_of_cores = settings.number_of_cores;
    settings_.max_payload_size = settings.max_payload_size;
    config->on_init(&settings_);
    return WEBRTC_VIDEO_CODEC_OK;
}

int32_t IVideoEncoder::RegisterEncodeCompleteCallback(
                                                      webrtc::EncodedImageCallback* callback)
{
    _callback = callback;
    return WEBRTC_VIDEO_CODEC_OK;
}

int32_t IVideoEncoder::Encode(const webrtc::VideoFrame& frame,
                              const std::vector<webrtc::VideoFrameType>* frame_types)
{
    auto c_frame = into_c((webrtc::VideoFrame*)&frame);
    auto types = (VideoFrameType*)frame_types->data();
    config->on_encode(c_frame, types, frame_types->size());
    return WEBRTC_VIDEO_CODEC_OK;
}

void IVideoEncoder::SetRates(
                             const webrtc::VideoEncoder::RateControlParameters& parameters)
{
    
}

int32_t IVideoEncoder::Release()
{
    return WEBRTC_VIDEO_CODEC_OK;
}

/* Video Encoder Factory */

std::unique_ptr<IVideoEncoderFactory> IVideoEncoderFactory::Create(std::vector<VideoEncoderConfig*> configs)
{
    auto self = std::make_unique<IVideoEncoderFactory>();
    self->_configs = configs;
    return self;
}

std::vector<webrtc::SdpVideoFormat> IVideoEncoderFactory::GetSupportedFormats() const
{
    return (*_factory)->GetSupportedFormats();
}

std::unique_ptr<webrtc::VideoEncoder> IVideoEncoderFactory::CreateVideoEncoder(const webrtc::SdpVideoFormat& format)
{
    for (auto config: _configs) {
        if (config->name == format.name.c_str()) {
            return IVideoEncoder::Create(config);;
        }
    }
    
    if (!_factory)
    {
        _factory = webrtc::CreateBuiltinVideoEncoderFactory();
    }
    
    return (*_factory)->CreateVideoEncoder(format);
}

VideoEncoderFactory* rtc_create_video_encoder_factory(VideoEncoderConfig** configs, size_t len)
{
    VideoEncoderFactory* v_factory = (VideoEncoderFactory*)malloc(sizeof(VideoEncoderFactory));
    if (!v_factory)
    {
        return NULL;
    }
    
    std::vector<VideoEncoderConfig*> configs_;
    for (size_t i = 0; i < len; i++)
    {
        configs_.push_back(configs[i]);
    }
    
    v_factory->factory = IVideoEncoderFactory::Create(configs_);
    return v_factory;
}
