//
//  video_decoder.h
//  rtc
//
//  Created by Mr.Panda on 2023/3/15.
//

#ifndef librtc_video_decoder_h
#define librtc_video_decoder_h
#pragma once

#include "api/video_codecs/video_decoder_factory.h"

class IVideoDecoderFactory
: public webrtc::VideoDecoderFactory
{
public:
    IVideoDecoderFactory();
    static std::unique_ptr<IVideoDecoderFactory> Create();
    std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const;
    std::unique_ptr<webrtc::VideoDecoder> CreateVideoDecoder(const webrtc::SdpVideoFormat& format);
private:
    std::unique_ptr<webrtc::VideoDecoderFactory> _factory;
    std::vector<webrtc::SdpVideoFormat> _formats;
};

#endif /* librtc_video_decoder_h */
