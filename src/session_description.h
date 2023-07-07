//
//  session_description.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef LIBRTC_SESSION_DESCRIPTION_H
#define LIBRTC_SESSION_DESCRIPTION_H
#pragma once

#include "api/peer_connection_interface.h"
#include "rtc.h"

webrtc::SdpType from_c(RTCSessionDescriptionType type);
std::unique_ptr<webrtc::SessionDescriptionInterface> from_c(RTCSessionDescription* desc);
RTCSessionDescription* into_c(webrtc::SessionDescriptionInterface* raw);
void free_session_description(RTCSessionDescription* raw);

#endif  // LIBRTC_SESSION_DESCRIPTION_H
