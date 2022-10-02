#include "ScriptHandler.h"
#include "../dev/Log.hpp"
#include "../application/SceneHandler.hpp"
#include "../application/Input.hpp"
#include "../application/Time.hpp"
#include "dev/LuaHelper.hpp"

#include "wrappers/SceneLua.h"
#include "wrappers/InputLua.h"

void ScriptHandler::lua_openmetatables(lua_State* L)
{
	LUA_ERR_CHECK(L, luaL_loadfile(L, (SCRIPT_PATH + "metatables/vector.lua").c_str()));
	LUA_ERR_CHECK(L, lua_pcall(L, 0, 1, 0));
	lua_setglobal(L, "vector");

	LUA_ERR_CHECK(L, luaL_loadfile(L, (SCRIPT_PATH + "core.lua").c_str()));
	LUA_ERR_CHECK(L, lua_pcall(L, 0, 1, 0));
	lua_setglobal(L, "core");
}

void ScriptHandler::updateScripts()
{
	entt::registry& reg = this->sceneHandler->getScene()->getSceneReg();
	auto view = reg.view<Transform, Behaviour>();

	auto func = [&](Transform& transform, const Behaviour& script)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, script.luaRef);
		if (luaL_dofile(L, script.path) != LUA_OK)
		{
			LuaH::dumpError(L);
		}
		else
		{
			lua_getfield(L, -1, "update"); // Get new update function
			lua_setfield(L, -3, "update"); // Set instance update function to the new one
			lua_pop(L, 1);
		}

		lua_getfield(L, -1, "update");
		if (lua_type(L, -1) == LUA_TNIL)
		{
			lua_pop(L, 1);
			return;
		}

		lua_pushvalue(L, -2);
		lua_pushnumber(L, Time::getDT());

		// Update function
		LUA_ERR_CHECK(L, lua_pcall(L, 2, 0, 0));

		lua_getfield(L, -1, "transform");
		Transform transformLua = lua_totransform(L, -1);
		lua_pop(L, 2);

		transform = transformLua;
	};
	view.each(func);
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
	this->updateScripts();

	if (Input::isKeyPressed(Keys::R))
	{
		this->sceneHandler->reloadScene();
	}
}

void ScriptHandler::cleanup()
{
	lua_close(L);
}
