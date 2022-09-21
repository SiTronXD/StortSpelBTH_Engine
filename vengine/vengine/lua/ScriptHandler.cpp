#include "ScriptHandler.h"
#include "../dev/Log.hpp"

ScriptHandler::ScriptHandler()
{
	L = luaL_newstate();
	luaL_openlibs(L);
	lua_pushstring(L, "Hello from lua!");
}

ScriptHandler::~ScriptHandler()
{
	lua_close(L);
}

void ScriptHandler::update()
{
	std::string str = lua_tostring(L, -1);
	luaL_dostring(L, ("print('" + str + "')").c_str());
}
