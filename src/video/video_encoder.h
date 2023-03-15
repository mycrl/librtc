//
//  video_encoder.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef librtc_video_encoder_h
#define librtc_video_encoder_h
#pragma once

#include "api/video_codecs/video_encoder_factory.h"
#include <optional>

class IVideoEncoderFactory
: public webrtc::VideoEncoderFactory
{
public:
    IVideoEncoderFactory();
    static std::unique_ptr<IVideoEncoderFactory> Create();
    std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const;
    std::unique_ptr<webrtc::VideoEncoder> CreateVideoEncoder(const webrtc::SdpVideoFormat& format);
    
    std::optional<std::unique_ptr<webrtc::VideoEncoderFactory>> _factory = nullptr;
    std::vector<webrtc::SdpVideoFormat> _formats;
};

#endif  // librtc_video_encoder_h
