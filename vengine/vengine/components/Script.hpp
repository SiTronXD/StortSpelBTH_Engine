#pragma once 
 #include "op_overload.hpp"

#include <cstring>

struct Script
{
	char path[64];
	int luaRef;

	Script(const char* path, int luaRef) : luaRef(luaRef)
	{
		memset(this->path, '\0', 64);
		strcpy(this->path, path);
	}
};