//
//  thread.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef LIBRTC_THREAD_H
#define LIBRTC_THREAD_H
#pragma once

#include "base.h"
#include "rtc_base/thread.h"

class IThread : public rtc::Thread
{

};

rtc::Thread* create_thread()
{
    
}

#endif // LIBRTC_THREAD_H