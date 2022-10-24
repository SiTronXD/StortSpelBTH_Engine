#include "../../application/Input.hpp"
#include "../../application/SceneHandler.hpp"
#include "../../application/Time.hpp"
#include "../../dev/Log.hpp"
#include "ServerScriptHandler.h"
#include "../../lua/dev/LuaHelper.hpp"

#include "../../lua/wrappers/InputLua.h" 
#include "NetworkScene.h"
#include "NetworkSceneLua.h"

void ServerScriptHandler::lua_openmetatables(lua_State* L)
{
	LUA_ERR_CHECK(
	    L, luaL_loadfile(L, (SCRIPT_PATH + "metatables/vector.lua").c_str())
	);
	LUA_ERR_CHECK(L, lua_pcall(L, 0, 1, 0));
	lua_setglobal(L, "vector");

	LUA_ERR_CHECK(L, luaL_loadfile(L, (SCRIPT_PATH + "core.lua").c_str()));
	LUA_ERR_CHECK(L, lua_pcall(L, 0, 1, 0));
	lua_setglobal(L, "core");
}

void ServerScriptHandler::updateScripts()
{
	entt::registry& reg = this->networkScene->getSceneReg();
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
			lua_setfield(
			    L, -3, "update"
			);  // Set instance update function to the new one
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
		lua_pushnumber(L, Time::getDT());

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

ServerScriptHandler::ServerScriptHandler()
{
	L = luaL_newstate();
	luaL_openlibs(L);
	lua_openmetatables(L);

	InputLua::lua_openinput(L);
}

ServerScriptHandler::~ServerScriptHandler() {}

void ServerScriptHandler::setScene(NetworkScene* scene)
{
	this->networkScene = scene;
	NetworkSceneLua::lua_openscene(L, scene);
}

bool ServerScriptHandler::runScript(std::string& path)
{
	bool result = luaL_dofile(L, path.c_str()) == LUA_OK;
	if (!result)
	{
		LuaH::dumpError(L);
	}
	return result;
}

void ServerScriptHandler::setScriptComponent(Entity entity, std::string& path)
{
	if (luaL_dofile(L, path.c_str()) != LUA_OK)
	{
		LuaH::dumpError(L);
	}
	else
	{
		lua_pushvalue(L, -1);
		int luaRef = luaL_ref(L, LUA_REGISTRYINDEX);

		lua_pushinteger(L, entity);
		lua_setfield(L, -2, "ID");

		lua_pushstring(L, path.c_str());
		lua_setfield(L, -2, "path");

		Transform& t = networkScene->getComponent<Transform>(entity);
		lua_pushtransform(L, networkScene->getComponent<Transform>(entity));
		lua_setfield(L, -2, "transform");

		networkScene->setComponent<Script>(entity, path.c_str(), luaRef);

		lua_getfield(L, -1, "init");
		if (lua_type(L, -1) == LUA_TNIL)
		{
			lua_pop(L, 2);
		}

		lua_pushvalue(L, -2);
		if (lua_pcall(L, 1, 0, 0) != LUA_OK)
		{
			LuaH::dumpError(L);
		}
		else
		{
			lua_getfield(L, -1, "transform");
			if (!lua_isnil(L, -1))
			{
				networkScene->setComponent<Transform>(
				    entity, lua_totransform(L, -1)
				);
			}
			lua_pop(L, 1);
		}
		lua_pop(L, 1);
	}
}

void ServerScriptHandler::updateSystems(std::vector<LuaSystem>& vec)
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
			lua_pushnumber(L, Time::getDT());
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

void ServerScriptHandler::update()
{
	this->updateScripts();
}

void ServerScriptHandler::cleanup()
{
	lua_close(L);
}

bool ServerScriptHandler::getScriptComponentValue(
    Script& script, int& ret, std::string name
)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, script.luaRef);
	lua_getfield(L, -1, name.c_str());

	bool result = lua_isnumber(L, -1);
	if (result)
	{
		ret = (int)lua_tointeger(L, -1);
	}
	return result;
}

bool ServerScriptHandler::getScriptComponentValue(
    Script& script, float& ret, std::string name
)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, script.luaRef);
	lua_getfield(L, -1, name.c_str());

	bool result = lua_isnumber(L, -1);
	if (result)
	{
		ret = (float)lua_tonumber(L, -1);
	}
	return result;
}

bool ServerScriptHandler::getScriptComponentValue(
    Script& script, std::string& ret, std::string name
)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, script.luaRef);
	lua_getfield(L, -1, name.c_str());

	bool result = lua_isstring(L, -1);
	if (result)
	{
		ret = lua_tostring(L, -1);
	}
	return result;
}