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
#include "audio_device_module.h"
#include "peer_connection_config.h"
#include "video_encoder.h"
#include "video_decoder.h"
#include "observer.h"
#include "thread.h"
#include "rtc.h"

void rtc_close(RTCPeerConnection* rtc)
{
    assert(rtc);

    rtc->pc->Close();
    rtc::CleanupSSL();
    delete rtc;
}

RTCPeerConnection* rtc_create_peer_connection(RTCPeerConnectionConfigure* c_config,
                                              Events* events,
                                              void* ctx)
{
    assert(c_config);
    assert(events);

    rtc::InitializeSSL();

    RTCPeerConnection* rtc = new RTCPeerConnection();
    rtc->threads = RtcThreads::Create();
    rtc->pc_factory = webrtc::CreatePeerConnectionFactory(rtc->threads->network_thread.get(),
                                                          rtc->threads->worker_thread.get(),
                                                          rtc->threads->signaling_thread.get(),
                                                          IAudioDeviceModule::Create(),
                                                          webrtc::CreateBuiltinAudioEncoderFactory(),
                                                          webrtc::CreateBuiltinAudioDecoderFactory(),
                                                          IVideoEncoderFactory::Create(),
                                                          IVideoDecoderFactory::Create(),
                                                          nullptr,
                                                          nullptr);
    if (!rtc->pc_factory)
    {
        delete rtc;
        return nullptr;
    }

    webrtc::PeerConnectionDependencies pc_dependencies(Observer::Create(events, ctx));
    auto ret = rtc->pc_factory->CreatePeerConnectionOrError(from_c(c_config),
                                                            std::move(pc_dependencies));
    if (ret.ok())
    {
        rtc->pc = std::move(ret.value());
    }
    else
    {
        delete rtc;
        return nullptr;
    }

    return rtc;
}

bool rtc_add_ice_candidate(RTCPeerConnection* rtc, RTCIceCandidate* icecandidate)
{
    assert(rtc);
    assert(icecandidate);

    return rtc->pc->AddIceCandidate(from_c(icecandidate));
}

void rtc_create_answer(RTCPeerConnection* rtc, CreateDescCallback callback, void* ctx)
{
    assert(rtc);
    assert(callback);

    auto opt = webrtc::PeerConnectionInterface::RTCOfferAnswerOptions();
    rtc->pc->CreateAnswer(CreateDescObserver::Create(callback, ctx), opt);
}

void rtc_create_offer(RTCPeerConnection* rtc, CreateDescCallback callback, void* ctx)
{
    assert(rtc);
    assert(callback);

    auto opt = webrtc::PeerConnectionInterface::RTCOfferAnswerOptions();
    rtc->pc->CreateOffer(CreateDescObserver::Create(callback, ctx), opt);
}

void rtc_set_local_description(RTCPeerConnection* rtc,
                               RTCSessionDescription* c_desc,
                               SetDescCallback callback,
                               void* ctx)
{
    assert(rtc);
    assert(c_desc);
    assert(callback);

    auto observer = SetDescObserver::Create(callback, ctx);
    rtc->pc->SetLocalDescription(std::move(observer), from_c(c_desc).release());
}

void rtc_set_remote_description(RTCPeerConnection* rtc,
                                RTCSessionDescription* c_desc,
                                SetDescCallback callback,
                                void* ctx)
{
    assert(rtc);
    assert(c_desc);
    assert(callback);

    auto observer = SetDescObserver::Create(callback, ctx);
    rtc->pc->SetRemoteDescription(std::move(observer), from_c(c_desc).release());
}

int rtc_add_media_stream_track(RTCPeerConnection* rtc,
                               MediaStreamTrack* track,
                               char* stream_id)
{
    assert(rtc);
    assert(track);
    assert(stream_id);

    webrtc::RTCErrorOr<rtc::scoped_refptr<webrtc::RtpSenderInterface>> ret;
    if (track->kind == MediaStreamTrackKind::MediaStreamTrackKindVideo)
    {
        ret = rtc->pc->AddTrack(
            std::move(rtc->pc_factory->CreateVideoTrack(track->label,
                                                        track->video_source)),
            { stream_id });
    }
    else
    {
        ret = rtc->pc->AddTrack(
            std::move(rtc->pc_factory->CreateAudioTrack(track->label,
                                                        track->audio_source)),
            { stream_id });
    }

    if (ret.ok())
    {
        track->sender = std::optional(std::move(ret.value()));
        return 0;
    }
    else
    {
        return -1;
    }
}

int rtc_remove_media_stream_track(RTCPeerConnection* rtc, MediaStreamTrack* track)
{
    assert(rtc);
    assert(track);

    if (!track->sender.has_value())
    {
        return -1;
    }

    auto sender = track->sender.value();
    auto ret = rtc->pc->RemoveTrackOrError(sender);
    return ret.ok() ? 0 : -2;
}

RTCDataChannel* rtc_create_data_channel(RTCPeerConnection* rtc,
                                        char* label,
                                        DataChannelOptions* options)
{
    assert(rtc);
    assert(label);
    assert(options);

    auto init = from_c(options);
    auto ret = rtc->pc->CreateDataChannelOrError(std::string(label), std::move(init));
    if (ret.ok())
    {
        return create_data_channel(std::move(ret.value()));
    }
    else
    {
        return nullptr;
    }
}
