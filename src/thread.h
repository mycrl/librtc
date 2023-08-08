//
//  thread.h
//  rtc
//
//  Created by Mr.Panda on 2023/8/8.
//

#ifndef LIBRTC_THREAD_H
#define LIBRTC_THREAD_H
#pragma once

#include "rtc.h"

RtcThreads* create_threads();
void close_threads(RtcThreads* threads);

#endif // LIBRTC_THREAD_H