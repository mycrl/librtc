//
//  base.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#include <cstdlib>
#include "base.h"

void free_incomplete_ptr(void*& ptr)
{
    if (ptr != nullptr)
    {
        delete ptr;
        ptr = nullptr;
    }
}

char* copy_c_str(std::string& source)
{
    char* dst = new char[source.size() + 1];
    strcpy(dst, source.c_str());
    return dst;
}