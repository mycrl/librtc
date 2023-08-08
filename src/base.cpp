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

char* copy_c_str(std::string& source)
{
	char* c_str = (char*)malloc(sizeof(char) * source.size() + 1);
	return strcpy(c_str, source.c_str());
}