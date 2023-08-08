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
    rtc::Thread* GetWorkThread();
    rtc::Thread* GetNetworkThread();
    rtc::Thread* GetSignalingThread();
private:
    std::unique_ptr<rtc::Thread> _work_thread;
    std::unique_ptr<rtc::Thread> _network_thread;
    std::unique_ptr<rtc::Thread> _signaling_thread;
};

#endif // LIBRTC_THREAD_H