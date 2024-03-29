//
//  video_encoder.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "video_encoder.h"
#include "h264_encoder.h"

IVideoEncoderFactory::IVideoEncoderFactory()
{
	for (auto formats : { H264Encoder::GetSupportedFormats() })
	{
		for (auto format : formats)
		{
			_formats.push_back(format);
		}
	}
}

std::unique_ptr<IVideoEncoderFactory> IVideoEncoderFactory::Create()
{
	return std::make_unique<IVideoEncoderFactory>();
}

std::vector<webrtc::SdpVideoFormat> IVideoEncoderFactory::GetSupportedFormats() const
{
	return _formats;
}

std::unique_ptr<webrtc::VideoEncoder> IVideoEncoderFactory::CreateVideoEncoder(const webrtc::SdpVideoFormat& format)
{
	return H264Encoder::Create(format);
}
