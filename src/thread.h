//
//  thread.h
//  rtc
//
//  Created by Mr.Panda on 2023/8/8.
//

#ifndef LIBRTC_THREAD_H
#define LIBRTC_THREAD_H
#pragma once

#include "rtc_base/thread.h"
#include "rtc.h"

#include <memory>

class RtcThreads
{
public:
    RtcThreads();
    ~RtcThreads();

    static std::unique_ptr<RtcThreads> Create();

    std::unique_ptr<rtc::Thread> work_thread;
    std::unique_ptr<rtc::Thread> network_thread;
    std::unique_ptr<rtc::Thread> signaling_thread;
};

#endif // LIBRTC_THREAD_H