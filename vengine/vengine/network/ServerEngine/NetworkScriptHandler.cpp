#include "NetworkScriptHandler.h"
#include "../../application/Input.hpp"
#include "../../application/SceneHandler.hpp"
#include "../../application/Time.hpp"
#include "../../dev/Log.hpp"
#include "../../lua/dev/LuaHelper.hpp"
#include "../../lua/wrappers/InputLua.h"
#include "NetworkScene.h"
#include "NetworkSceneLua.h"

void NetworkScriptHandler::updateScripts(float dt)
{
	entt::registry& reg = sceneHandler->getScene()->getSceneReg();
	auto view = reg.view<Transform, Script>(entt::exclude<Inactive>);

	auto func = [&](Transform& transform, const Script& script)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, script.luaRef);
		if (luaL_dofile(L, script.path) != LUA_OK)
		{
			LuaH::dumpError(L);
		}
		else
		{
			lua_getfield(L, -1, "update");  // Get new update function
			lua_setfield(L, -3, "update");  // Set instance update function to the new one
			lua_pop(L, 1);
		}

		lua_getfield(L, -1, "update");
		if (lua_type(L, -1) == LUA_TNIL)
		{
			lua_pop(L, 1);
			return;
		}

		lua_pushtransform(L, transform);
		lua_setfield(L, -3, "transform");

		lua_pushvalue(L, -2);
		lua_pushnumber(L, dt);

		// Update function
		LUA_ERR_CHECK(L, lua_pcall(L, 2, 0, 0));

		lua_getfield(L, -1, "transform");
		if (!lua_isnil(L, -1))
		{
			transform = lua_totransform(L, -1);
		}
		lua_pop(L, 2);
	};
	view.each(func);
}

NetworkScriptHandler::NetworkScriptHandler()
{
	ScriptHandler::ScriptHandler();
}

NetworkScriptHandler::~NetworkScriptHandler() {}

void NetworkScriptHandler::updateSystems(std::vector<LuaSystem>& vec, float dt)
{
	for (auto it = vec.begin(); it != vec.end();)
	{
		if (luaL_dofile(L, (*it).path.c_str()) != LUA_OK)
		{
			LuaH::dumpError(L);
			it++;
			continue;
		}

		// Add lua reference
		if ((*it).luaRef == -1)
		{
			lua_pushvalue(L, -1);
			(*it).luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
		}
		lua_rawgeti(L, LUA_REGISTRYINDEX, (*it).luaRef);

		lua_getfield(L, -2, "update");  // Get new update
		if (!lua_isnil(L, -1))          // Found update
		{
			lua_pushvalue(L, -2);
			lua_pushnumber(L, dt);
			if (lua_pcall(L, 2, 1, 0) != LUA_OK)
			{
				LuaH::dumpError(L);
				it++;
				lua_pop(L, 2);
				continue;
			}
			bool returned = lua_toboolean(L, -1);
			if (returned)
			{
				it = vec.erase(it);
			}
			else
			{
				it++;
			}
			lua_pop(L, 3);
		}
		else
		{
			it++;
		}
	}
}

void NetworkScriptHandler::update(float dt)
{
	this->updateScripts(dt);

	// Empty stack if needed
	if (lua_gettop(L))
	{
		lua_pop(L, lua_gettop(L));
	}

	if (Input::isKeyPressed(Keys::R) && Input::isKeyDown(Keys::CTRL))
	{
		this->sceneHandler->reloadScene();
	}
}

