//
//  observer.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef LIBRTC_OBSERVER_H
#define LIBRTC_OBSERVER_H
#pragma once

#include "api/peer_connection_interface.h"
#include "session_description.h"
#include "media_stream_track.h"
#include "ice_candidate.h"
#include "data_channel.h"
#include "rtc.h"

class Observer
    : public webrtc::PeerConnectionObserver
    , public rtc::RefCountInterface
{
public:
    Observer(Events* events, void* ctx);

    static Observer* Create(Events* events, void* ctx);
    void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState state);
    void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel);
    void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState state);
    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);
    void OnRenegotiationNeeded();
    void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState state);
    void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver);
    void OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState state);
private:
    Events* _events = nullptr;
    void* _ctx = nullptr;
};

class CreateDescObserver
    : public webrtc::CreateSessionDescriptionObserver
{
public:
    CreateDescObserver(CreateDescCallback callback, void* ctx);

    static CreateDescObserver* Create(CreateDescCallback callback, void* ctx);

    // This callback transfers the ownership of the `desc`.
    // TODO(deadbeef): Make this take an std::unique_ptr<> to avoid confusion
    // around ownership.
    void OnSuccess(webrtc::SessionDescriptionInterface* desc);

    // The OnFailure callback takes an RTCError, which consists of an
    // error code and a string.
    // RTCError is non-copyable, so it must be passed using std::move.
    // Earlier versions of the API used a string argument. This version
    // is removed; its functionality was the same as passing
    // error.message.
    void OnFailure(webrtc::RTCError error);
private:
    CreateDescCallback _callback = nullptr;
    void* _ctx = nullptr;
};

class SetDescObserver
    : public webrtc::SetSessionDescriptionObserver
{
public:
    SetDescObserver(SetDescCallback callback, void* ctx);

    static SetDescObserver* Create(SetDescCallback callback, void* ctx);
    virtual void OnSuccess();

    // See description in CreateSessionDescriptionObserver for OnFailure.
    virtual void OnFailure(webrtc::RTCError error);
private:
    SetDescCallback _callback = nullptr;
    void* _ctx = nullptr;
};

PeerConnectionState into_c(webrtc::PeerConnectionInterface::PeerConnectionState state);
SignalingState into_c(webrtc::PeerConnectionInterface::SignalingState state);
IceGatheringState into_c(webrtc::PeerConnectionInterface::IceGatheringState state);
IceConnectionState into_c(webrtc::PeerConnectionInterface::IceConnectionState state);

#endif  // LIBRTC_OBSERVER_H
