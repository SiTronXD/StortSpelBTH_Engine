#pragma once

#include <cstring>

struct Script
{
	char path[64];
	int luaRef;

	Script(const char* path, int luaRef) : luaRef(luaRef)
	{
		memset(this->path, '\0', 64);
		strcpy_s(this->path, path);
	}
};