#pragma once

#include "dev/LuaHelper.h"

class ScriptHandler
{
private:
	lua_State* L;

public:
	ScriptHandler();
	virtual ~ScriptHandler();

	void update();
};

