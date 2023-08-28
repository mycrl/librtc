//
//  base.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef LIBRTC_BASE_H
#define LIBRTC_BASE_H
#pragma once

#ifdef WIN32
#define EXPORT __declspec(dllexport)
#endif

#ifdef WEBRTC_POSIX
#define EXPORT
#endif

#include <string>

template <typename T>
inline void free_ptr(T* ptr)
{
    if (ptr != nullptr)
    {
        delete ptr;
    }
}

char* copy_c_str(std::string& source);

#define PANIC(...) { \
    printf(__VA_ARGS__); \
    std::abort(); \
}

#endif  // LIBRTC_BASE_H
