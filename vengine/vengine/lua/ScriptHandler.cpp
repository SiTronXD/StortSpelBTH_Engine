#include "ScriptHandler.h"
#include "../dev/Log.hpp"
#include "../application/Input.hpp"
#include "dev/LuaHelper.hpp"

#include "wrappers/SceneLua.h"
#include "wrappers/InputLua.h"

void ScriptHandler::lua_openmetatables(lua_State* L)
{
	LUA_ERR_CHECK(L, luaL_loadfile(L, (SCRIPT_PATH + "metatables/vector.lua").c_str()));
	LUA_ERR_CHECK(L, lua_pcall(L, 0, 1, 0));
	lua_setglobal(L, "vector");
}

ScriptHandler::ScriptHandler()
{
	L = luaL_newstate();
	luaL_openlibs(L);
	lua_openmetatables(L);

	InputLua::lua_openinput(L);
}

ScriptHandler::~ScriptHandler()
{
	lua_close(L);
}

void ScriptHandler::setSceneHandler(SceneHandler* sceneHandler)
{
	this->sceneHandler = sceneHandler;
	SceneLua::lua_openscene(L, sceneHandler);
}

void ScriptHandler::update()
{
	if (Input::isKeyPressed(Keys::R))
	{
		LUA_ERR_CHECK(L, luaL_dofile(L, (SCRIPT_PATH + "test.lua").c_str()));
	}
}
