//
//  ice_candidate.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef LIBRTC_ICE_CANDIDATE_H
#define LIBRTC_ICE_CANDIDATE_H
#pragma once

#include "api/peer_connection_interface.h"
#include "rtc.h"

const webrtc::IceCandidateInterface* from_c(RTCIceCandidate* candidate);
RTCIceCandidate* into_c(webrtc::IceCandidateInterface* candidate);
void free_ice_candidate(RTCIceCandidate* candidate);

#endif  // LIBRTC_ICE_CANDIDATE_H
