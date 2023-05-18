//
//  base.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#include <cstdlib>
#include "base.h"

void free_incomplete_ptr(void* ptr)
{
	if (ptr)
	{
		free(ptr);
	}
}
