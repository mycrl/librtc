//
//  h264.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/3/10.
//

#include "h264.h"

webrtc::SdpVideoFormat create_h264_format(webrtc::H264Profile profile,
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

std::vector<webrtc::SdpVideoFormat> supported_h264_codecs(bool mode)
{
    return {create_h264_format(webrtc::H264Profile::kProfileBaseline, webrtc::H264Level::kLevel3_1, "1", mode),
        create_h264_format(webrtc::H264Profile::kProfileBaseline, webrtc::H264Level::kLevel3_1, "0", mode),
        create_h264_format(webrtc::H264Profile::kProfileConstrainedBaseline, webrtc::H264Level::kLevel3_1, "1", mode),
        create_h264_format(webrtc::H264Profile::kProfileConstrainedBaseline, webrtc::H264Level::kLevel3_1, "0", mode),
        create_h264_format(webrtc::H264Profile::kProfileMain, webrtc::H264Level::kLevel3_1, "1", mode),
        create_h264_format(webrtc::H264Profile::kProfileMain, webrtc::H264Level::kLevel3_1, "0", mode)};
}

CodecLayer find_codec(CodecKind kind)
{
    CodecLayer layer;
    for (auto name: Encoders)
    {
        layer.codec = (kind == CodecKind::kEncoder)
            ? avcodec_find_encoder_by_name(name.c_str())
            : avcodec_find_decoder_by_name(name.c_str());
        if (layer.codec)
        {
            layer.name = name;
            break;
        }
    }
    
    return layer;
}
