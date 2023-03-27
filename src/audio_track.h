//
//  audio_track.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef librtc_audio_track_h
#define librtc_audio_track_h
#pragma once

#include "api/media_stream_interface.h"
#include "frame.h"
#include "base.h"
#include <set>

/* audio source */

class IAudioTrackSource
: public webrtc::AudioSourceInterface
{
public:
    static IAudioTrackSource* Create();
    void RegisterObserver(webrtc::ObserverInterface* observer);
    void UnregisterObserver(webrtc::ObserverInterface* observer) {}
    void AddSink(webrtc::AudioTrackSinkInterface* sink);
    void RemoveSink(webrtc::AudioTrackSinkInterface* sink);
    const cricket::AudioOptions options() const;
    SourceState state() const;
    bool remote() const;
    void OnData(IAudioFrame* frame);
private:
    std::set<webrtc::AudioTrackSinkInterface*> _sinks;
};

/* audio sink */

class IAudioTrackSink
: public webrtc::AudioTrackSinkInterface
, public rtc::RefCountInterface
{
public:
    IAudioTrackSink(webrtc::AudioTrackInterface* track);
    static IAudioTrackSink* Create(webrtc::AudioTrackInterface* track);
    void SetOnFrame(void* ctx, void(*handler)(void* ctx, IAudioFrame* frame));
    void RemoveOnFrame();
    void OnData(const void* audio_data,
                int bits_per_sample,
                int sample_rate,
                size_t number_of_channels,
                size_t number_of_frames,
                absl::optional<int64_t> capture_timestamp_ms);
private:
    void(*_handler)(void* ctx, IAudioFrame* frame) = NULL;
    webrtc::AudioTrackInterface* _track = NULL;
    void* _ctx = NULL;
};

#endif /* librtc_audio_track_h */
