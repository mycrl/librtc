//
//  peer_connection.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include "rtc_base/ssl_adapter.h"
#include "audio_capture_module.h"
#include "peer_connection_config.h"
#include "video_encoder.h"
#include "video_decoder.h"
#include "observer.h"
#include "rtc.h"

void rtc_run()
{
    rtc::InitializeSSL();
    rtc::Thread::Current()->Run();
}

void rtc_exit()
{
    rtc::CleanupSSL();
    rtc::Thread::Current()->Quit();
}

void rtc_close(RTCPeerConnection* rtc)
{
    rtc->pc->Close();
    delete rtc;
}

RTCPeerConnection* rtc_create_peer_connection(RTCPeerConnectionConfigure* c_config,
                                              Events* events,
                                              void* ctx)
{
    RTCPeerConnection* rtc = new RTCPeerConnection();
    rtc->pc_factory = webrtc::CreatePeerConnectionFactory(nullptr,
                                                          nullptr,
                                                          nullptr,
                                                          AudioCaptureModule::Create(),
                                                          webrtc::CreateBuiltinAudioEncoderFactory(),
                                                          webrtc::CreateBuiltinAudioDecoderFactory(),
                                                          IVideoEncoderFactory::Create(),
                                                          IVideoDecoderFactory::Create(),
                                                          nullptr,
                                                          nullptr);
    if (!rtc->pc_factory)
    {
        return NULL;
    }

    webrtc::PeerConnectionDependencies pc_dependencies(Observer::Create(events, ctx));
    auto error_or_pc = rtc->pc_factory->CreatePeerConnectionOrError(from_c(c_config),
                                                                    std::move(pc_dependencies));
    if (error_or_pc.ok())
    {
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
    rtc->pc->SetLocalDescription(std::move(observer), from_c(c_desc).release());
}

void rtc_set_remote_description(RTCPeerConnection* rtc,
                                RTCSessionDescription* c_desc,
                                SetDescCallback callback,
                                void* ctx)
{
    auto observer = SetDescObserver::Create(callback, ctx);
    rtc->pc->SetRemoteDescription(std::move(observer), from_c(c_desc).release());
}

void rtc_add_media_stream_track(RTCPeerConnection* rtc,
                                MediaStreamTrack* target,
                                char* stream_id)
{
    if (target->kind == MediaStreamTrackKind::MediaStreamTrackKindVideo)
    {
        auto track = rtc->pc_factory->CreateVideoTrack(target->label, target->video_source);
        rtc->pc->AddTrack(std::move(track), { stream_id });
    }
    else
    {
        auto track = rtc->pc_factory->CreateAudioTrack(target->label, target->audio_source);
        rtc->pc->AddTrack(std::move(track), { stream_id });
    }
}

RTCDataChannel* rtc_create_data_channel(RTCPeerConnection* rtc,
                                        char* label,
                                        DataChannelOptions* options)
{
    auto init = from_c(options);
    auto ret = rtc->pc->CreateDataChannelOrError(std::string(label), std::move(init));
    if (ret.ok())
    {
        return create_data_channel(std::move(ret.value()));
    }
    else
    {
        return NULL;
    }
}
