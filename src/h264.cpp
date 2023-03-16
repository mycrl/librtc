//
//  h264.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/3/10.
//

#include "h264.h"

webrtc::SdpVideoFormat ICreateH264Format(webrtc::H264Profile profile,
                                         webrtc::H264Level level,
                                         const std::string& packetization_mode,
                                         bool add_scalability_modes)
{
    const absl::optional<std::string> profile_string =
        H264ProfileLevelIdToString(webrtc::H264ProfileLevelId(profile, level));
    absl::InlinedVector<webrtc::ScalabilityMode, webrtc::kScalabilityModeCount> scalability_modes;
    
    if (add_scalability_modes)
    {
        for (const auto scalability_mode : IkSupportedScalabilityModes)
        {
            scalability_modes.push_back(scalability_mode);
        }
    }
    
    return webrtc::SdpVideoFormat(
                                  cricket::kH264CodecName,
                                  {{cricket::kH264FmtpProfileLevelId, *profile_string},
                                      {cricket::kH264FmtpLevelAsymmetryAllowed, "1"},
                                      {cricket::kH264FmtpPacketizationMode, packetization_mode}},
                                  scalability_modes);
}

std::vector<webrtc::SdpVideoFormat> ISupportedH264Codecs(bool mode /* add_scalability_modes */)
{
    return {ICreateH264Format(webrtc::H264Profile::kProfileBaseline, webrtc::H264Level::kLevel3_1, "1", mode),
        ICreateH264Format(webrtc::H264Profile::kProfileBaseline, webrtc::H264Level::kLevel3_1, "0", mode),
        ICreateH264Format(webrtc::H264Profile::kProfileConstrainedBaseline, webrtc::H264Level::kLevel3_1, "1", mode),
        ICreateH264Format(webrtc::H264Profile::kProfileConstrainedBaseline, webrtc::H264Level::kLevel3_1, "0", mode),
        ICreateH264Format(webrtc::H264Profile::kProfileMain, webrtc::H264Level::kLevel3_1, "1", mode),
        ICreateH264Format(webrtc::H264Profile::kProfileMain, webrtc::H264Level::kLevel3_1, "0", mode)};
}
