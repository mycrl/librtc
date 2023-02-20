//
//  audio_track.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#include "audio_track.h"

/*
 IAudioTrackSink
 */

IAudioTrackSink::IAudioTrackSink(webrtc::AudioTrackInterface* track)
{
    _track = track;
    _track->AddRef();
}

IAudioTrackSink* IAudioTrackSink::Create(webrtc::AudioTrackInterface* track)
{
    auto self = new rtc::RefCountedObject<IAudioTrackSink>(track);
    self->AddRef();
    return self;
}

void IAudioTrackSink::OnData(const void* audio_data,
                             int bits_per_sample,
                             int sample_rate,
                             size_t number_of_channels,
                             size_t number_of_frames)
{
    if (!_handler || !audio_data)
    {
        return;
    }
    
    auto frames = into_c((const uint8_t*)audio_data,
                         bits_per_sample,
                         sample_rate,
                         number_of_channels,
                         number_of_frames);
    _handler(_ctx, frames);
}

void IAudioTrackSink::SetOnFrame(void* ctx,
                                 void(*handler)(void* ctx, IAudioFrame* frame))
{
    _track->AddSink(this);
    _handler = handler;
    _ctx = ctx;
}

void IAudioTrackSink::RemoveOnFrame()
{
    _track->RemoveSink(this);
    _handler = NULL;
    _ctx = NULL;
}

/*
 IAudioTrackSource
 */

void IAudioTrackSource::AddSink(webrtc::AudioTrackSinkInterface* sink)
{
    _sinks.push_back(sink);
}

void IAudioTrackSource::RemoveSink(webrtc::AudioTrackSinkInterface* sink)
{
    auto it = std::find(_sinks.begin(), _sinks.end(), sink);
    if (it != _sinks.end())
    {
        _sinks.erase(it);
    }
}

void IAudioTrackSource::AddData(const uint16_t* buf, size_t size, size_t frames_size)
{
    for (auto sink: _sinks)
    {
        sink->OnData(buf,
                     bits_per_sample,
                     sample_rate,
                     number_of_channels,
                     frames_size);
    }
}
