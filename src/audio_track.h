//
//  audio_track.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef audio_track_h
#define audio_track_h
#pragma once

#include "api/media_stream_interface.h"
#include "frame.h"
#include "base.h"

/*
 audio source
 */
class IAudioTrackSource
: public rtc::RefCountedObject<webrtc::AudioSourceInterface>
{
public:
    void AddSink(webrtc::AudioTrackSinkInterface* sink);
    void RemoveSink(webrtc::AudioTrackSinkInterface* sink);
    void AddData(const uint16_t* buf, size_t size, size_t frames_size);
    
    size_t number_of_channels;
    int bits_per_sample;
    int sample_rate;
private:
    std::vector<webrtc::AudioTrackSinkInterface*> _sinks;
    std::vector<webrtc::ObserverInterface*> _observers;
};

/*
 audio sink
 */

class IAudioTrackSink
: public webrtc::AudioTrackSinkInterface
, public rtc::RefCountInterface
{
public:
    IAudioTrackSink(webrtc::AudioTrackInterface* track);
    static IAudioTrackSink* Create(webrtc::AudioTrackInterface* track);
    void OnData(const void* buf, int b, int s, size_t c, size_t f);
    void SetOnFrame(void* ctx, void(*handler)(void* ctx, IAudioFrame* frame));
    void RemoveOnFrame();
private:
    void(*_handler)(void* ctx, IAudioFrame* frame) = NULL;
    webrtc::AudioTrackInterface* _track = NULL;
    void* _ctx = NULL;
};

#endif /* audio_track_h */
