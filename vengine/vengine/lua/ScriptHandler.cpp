#include "ScriptHandler.h"
#include "../dev/Log.hpp"
#include "../application/SceneHandler.hpp"
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
}

void ScriptHandler::setSceneHandler(SceneHandler* sceneHandler)
{
	this->sceneHandler = sceneHandler;
	SceneLua::lua_openscene(L, sceneHandler);
}

bool ScriptHandler::runScript(std::string& path)
{
	bool result = luaL_dofile(L, path.c_str()) == LUA_OK;
	if (!result) { LuaH::dumpError(L); }
	return result;
}

bool ScriptHandler::loadScript(std::string& path)
{
	bool result = luaL_loadfile(L, path.c_str()) == LUA_OK;
	if (!result) { LuaH::dumpError(L); }
	return result;
}

void ScriptHandler::update()
{
	if (Input::isKeyPressed(Keys::R))
	{
		this->sceneHandler->reloadScene();
	}
}

void ScriptHandler::cleanup()
{
	lua_close(L);
}
