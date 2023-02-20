//
//  base.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#include "base.h"
#include <cstdlib>

void free_incomplete_ptr(void* ptr)
{
    if (ptr)
    {
        free(ptr);
    }
}
