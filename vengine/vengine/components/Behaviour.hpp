#pragma once

#include <cstring>

struct Behaviour
{
	char path[64];
	int luaRef;

	Behaviour(const char* path, int luaRef) : luaRef(luaRef)
	{
		memset(this->path, '\0', 64);
		strcpy_s(this->path, path);
	}
};