//
//  thread.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/8/8.
//

#include "thread.h"

RtcThreads* create_threads()
{
    auto threads = new RtcThreads;
    threads->work_thread = rtc::Thread::Create();
    threads->network_thread = rtc::Thread::CreateWithSocketServer();
    threads->signaling_thread = rtc::Thread::CreateWithSocketServer();
    threads->work_thread->Start();
    threads->network_thread->Start();
    threads->signaling_thread->Start();
    return threads;
}

void close_threads(RtcThreads* threads)
{
    threads->work_thread->Stop();
    threads->network_thread->Stop();
    threads->signaling_thread->Stop();
    delete threads;
}