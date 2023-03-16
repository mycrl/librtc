//
//  video_decoder.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/3/15.
//

#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "video_decoder.h"
#include "h264_decoder.h"

IVideoDecoderFactory::IVideoDecoderFactory()
: _factory(webrtc::CreateBuiltinVideoDecoderFactory())
{
    _formats = _factory->GetSupportedFormats();
    for (auto formats: {H264Decoder::GetSupportedFormats()})
    {
        for (auto format: formats)
        {
            _formats.push_back(format);
        }
    }
}

std::unique_ptr<IVideoDecoderFactory> IVideoDecoderFactory::Create()
{
    return std::make_unique<IVideoDecoderFactory>();
}

std::vector<webrtc::SdpVideoFormat> IVideoDecoderFactory::GetSupportedFormats() const
{
    return _formats;
}

std::unique_ptr<webrtc::VideoDecoder> IVideoDecoderFactory::CreateVideoDecoder(const webrtc::SdpVideoFormat& format)
{
    if (format.name == "H264")
    {
        return H264Decoder::Create();
    }
    
    return _factory->CreateVideoDecoder(format);
}
