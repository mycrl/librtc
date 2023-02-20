//
//  peer_connection.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include "rtc_base/ssl_adapter.h"
#include "peer_connection.h"

void rtc_run()
{
    rtc::InitializeSSL();
    rtc::Thread::Current()->Run();
}

void rtc_close(RTCPeerConnection* peer)
{
    peer->pc->Close();
    rtc::CleanupSSL();
    delete peer;
}

RTCPeerConnection* rtc_create_peer_connection(RTCPeerConnectionConfigure* c_config,
                                              Events* events,
                                              void* ctx)
{
    RTCPeerConnection* rtc = new RTCPeerConnection();
    if (!rtc)
    {
        return NULL;
    }
    
    rtc->pc_factory = webrtc::CreatePeerConnectionFactory(
                                                          nullptr,
                                                          nullptr,
                                                          nullptr,
                                                          nullptr,
                                                          webrtc::CreateBuiltinAudioEncoderFactory(),
                                                          webrtc::CreateBuiltinAudioDecoderFactory(),
                                                          webrtc::CreateBuiltinVideoEncoderFactory(),
                                                          webrtc::CreateBuiltinVideoDecoderFactory(),
                                                          nullptr,
                                                          nullptr);
    if (!rtc->pc_factory)
    {
        return NULL;
    }
    
    webrtc::PeerConnectionDependencies pc_dependencies(Observer::Create(events, ctx));
    auto error_or_pc = rtc->pc_factory->CreatePeerConnectionOrError(
                                                                    from_c(c_config), 
                                                                    std::move(pc_dependencies));
    if (error_or_pc.ok()) {
        rtc->pc = std::move(error_or_pc.value());
    }
    else
    {
        return NULL;
    }
    
    return rtc;
}

bool rtc_add_ice_candidate(RTCPeerConnection* rtc, RTCIceCandidate* icecandidate)
{
    return rtc->pc->AddIceCandidate(from_c(icecandidate));
}

void rtc_create_answer(RTCPeerConnection* rtc, CreateDescCallback callback, void* ctx)
{
    auto opt = webrtc::PeerConnectionInterface::RTCOfferAnswerOptions();
    rtc->pc->CreateAnswer(CreateDescObserver::Create(callback, ctx), opt);
}

void rtc_create_offer(RTCPeerConnection* rtc, CreateDescCallback callback, void* ctx)
{
    auto opt = webrtc::PeerConnectionInterface::RTCOfferAnswerOptions();
    rtc->pc->CreateOffer(CreateDescObserver::Create(callback, ctx), opt);
}

void rtc_set_local_description(RTCPeerConnection* rtc, 
                               RTCSessionDescription* c_desc, 
                               SetDescCallback callback, 
                               void* ctx)
{
    auto observer = SetDescObserver::Create(callback, ctx);
    rtc->pc->SetLocalDescription(observer, from_c(c_desc).release());
}

void rtc_set_remote_description(RTCPeerConnection* rtc,
                                RTCSessionDescription* c_desc,
                                SetDescCallback callback,
                                void* ctx)
{
    auto observer = SetDescObserver::Create(callback, ctx);
    rtc->pc->SetRemoteDescription(observer, from_c(c_desc).release());
}

void rtc_add_media_stream_track(RTCPeerConnection* rtc, MediaStreamTrack* track, char* stream_id)
{
    if (track->kind == MediaStreamTrackKind::MediaStreamTrackKindVideo)
    {
        auto video_track = rtc->pc_factory->CreateVideoTrack(track->label, track->video_source);
        rtc->pc->AddTrack(video_track, { stream_id });
    }
}

RTCDataChannel* rtc_create_data_channel(RTCPeerConnection* rtc,
                                        char* label,
                                        DataChannelOptions* options)
{
    auto init = from_c(options);
    auto ret = rtc->pc->CreateDataChannelOrError(std::string(label), std::move(init));
    if (ret.ok()) {
        return create_data_channel(std::move(ret.value()));
    }
    else
    {
        return NULL;
    }
}
