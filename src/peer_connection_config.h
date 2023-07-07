//
//  peer_connection_config.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef LIBRTC_PEER_CONNECTION_CONFIG_H
#define LIBRTC_PEER_CONNECTION_CONFIG_H
#pragma once

#include "api/peer_connection_interface.h"
#include "rtc.h"

std::vector<std::string> from_c(char** raw, int size);
webrtc::PeerConnectionInterface::IceServer from_c(RTCIceServer raw);
webrtc::PeerConnectionInterface::IceServers from_c(RTCIceServer* raw, int size);
webrtc::PeerConnectionInterface::RTCConfiguration from_c(RTCPeerConnectionConfigure* raw);

#endif  // LIBRTC_PEER_CONNECTION_CONFIG_H
