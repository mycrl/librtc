//
//  h264.h
//  rtc
//
//  Created by Mr.Panda on 2023/3/15.
//

#ifndef librtc_h264_h
#define librtc_h264_h
#pragma once

#include "modules/video_coding/codecs/h264/include/h264.h"

constexpr webrtc::ScalabilityMode IkSupportedScalabilityModes[] = {
    webrtc::ScalabilityMode::kL1T1,
    webrtc::ScalabilityMode::kL1T2,
    webrtc::ScalabilityMode::kL1T3
};

webrtc::SdpVideoFormat ICreateH264Format(webrtc::H264Profile profile,
                                         webrtc::H264Level level,
                                         const std::string& packetization_mode,
                                         bool add_scalability_modes);
std::vector<webrtc::SdpVideoFormat> ISupportedH264Codecs(bool mode /* add_scalability_modes */);

#endif /* librtc_h264_h */
