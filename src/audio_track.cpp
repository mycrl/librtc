//
//  audio_track.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#include "audio_track.h"

/* IAudioTrackSink */

IAudioTrackSink::IAudioTrackSink(webrtc::AudioTrackInterface* track) : _track(track)
{
    assert(track);
}

IAudioTrackSink* IAudioTrackSink::Create(webrtc::AudioTrackInterface* track)
{
    assert(track);

    return new rtc::RefCountedObject<IAudioTrackSink>(track);
}

void IAudioTrackSink::OnData(const void* audio_data,
                             int bits_per_sample,
                             int sample_rate,
                             size_t number_of_channels,
                             size_t number_of_frames,
                             absl::optional<int64_t> capture_timestamp_ms)
{
    if (!_handler.has_value() || !audio_data)
    {
        return;
    }

    auto frames = into_c((const int16_t*)audio_data,
                         sample_rate,
                         number_of_channels,
                         number_of_frames,
                         capture_timestamp_ms.value_or(0));
    _handler.value()(_ctx, frames);
}

void IAudioTrackSink::SetOnFrame(void* ctx,
                                 void(*handler)(void* ctx, IAudioFrame* frame))
{
    assert(handler);

    _track->AddSink(this);
    _handler = handler;
    _ctx = ctx;
}

void IAudioTrackSink::RemoveOnFrame()
{
    _track->RemoveSink(this);
    _handler = std::nullopt;
    _ctx = nullptr;
}

/* IAudioTrackSource */

IAudioTrackSource* IAudioTrackSource::Create()
{
    return new rtc::RefCountedObject<IAudioTrackSource>();
}

void IAudioTrackSource::RegisterObserver(webrtc::ObserverInterface* observer)
{
    assert(observer);

    observer->OnChanged();
}

void IAudioTrackSource::UnregisterObserver(webrtc::ObserverInterface* observer)
{
}

void IAudioTrackSource::AddSink(webrtc::AudioTrackSinkInterface* sink)
{
    assert(sink);

    _sinks.insert(sink);
}

void IAudioTrackSource::RemoveSink(webrtc::AudioTrackSinkInterface* sink)
{
    assert(sink);

    _sinks.erase(sink);
}

const cricket::AudioOptions IAudioTrackSource::options() const
{
    cricket::AudioOptions options;
#if defined(WEBRTC_IOS)
    options.ios_force_software_aec_HACK = true;
#endif
    options.echo_cancellation = true;
    options.auto_gain_control = true;
    options.noise_suppression = true;
    return options;
}

webrtc::AudioSourceInterface::SourceState IAudioTrackSource::state() const
{
    return SourceState::kLive;
}

bool IAudioTrackSource::remote() const
{
    return false;
}

void IAudioTrackSource::OnData(IAudioFrame* frame)
{
    assert(frame);

    for (auto& sink : _sinks)
    {
        sink->OnData(frame->data,
                     16,
                     frame->sample_rate,
                     frame->channels,
                     frame->frames,
                     frame->timestamp);
    }
}
