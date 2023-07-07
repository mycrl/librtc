//
//  media_stream_track.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef LIBRTC_MEDIA_STREAM_TRACK_H
#define LIBRTC_MEDIA_STREAM_TRACK_H
#pragma once

#include "video_track.h"
#include "audio_track.h"
#include "base.h"
#include "rtc.h"

MediaStreamTrack* from(webrtc::VideoTrackInterface* track);
MediaStreamTrack* from(webrtc::AudioTrackInterface* track);

#endif  // LIBRTC_MEDIA_STREAM_TRACK_H
