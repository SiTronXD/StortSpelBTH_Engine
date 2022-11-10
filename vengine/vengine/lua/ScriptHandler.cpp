#include "pch.h"
#include "ScriptHandler.h"
#include "../dev/Log.hpp"
#include "../application/SceneHandler.hpp"
#include "../network/NetworkHandler.h"
#include "../application/Input.hpp"
#include "../application/Time.hpp"
#include "dev/LuaHelper.hpp"

#include "wrappers/SceneLua.h"
#include "wrappers/InputLua.h"
#include "wrappers/ResourceManagerLua.h"
#include "wrappers/NetworkHandlerLua.h"
#include "wrappers/PhysicsEngineLua.h"
#include "wrappers/UIRendererLua.h"
#include "wrappers/DebugRendererLua.h"

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
	auto view = reg.view<Transform, Script>(entt::exclude<Inactive>);

	auto func = [&](Transform& transform, const Script& script)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, script.luaRef);
#ifdef _CONSOLE
		if (luaL_dofile(L, script.path) != LUA_OK)
		{
			LuaH::dumpError(L);
		}
		else
		{
			LuaH::replaceTableFunctions(L, -1, -2);
			lua_pop(L, 1);
		}
#endif
		lua_getfield(L, -1, "update");
		if (lua_type(L, -1) == LUA_TNIL)
		{
			lua_pop(L, 2);
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

void ScriptHandler::setResourceManager(ResourceManager* resourceManager)
{
	ResourceManagerLua::lua_openresources(L, resourceManager);
}

void ScriptHandler::setNetworkHandler(NetworkHandler* networkHandler) 
{
	this->networkHandler = networkHandler;
	NetworkHandlerLua::lua_openNetworkScene(L, networkHandler);
}

void ScriptHandler::setPhysicsEngine(PhysicsEngine* physicsEngine)
{
	PhysicsEngineLua::lua_openphysics(L, physicsEngine);
}

void ScriptHandler::setUIRenderer(UIRenderer* uiRenderer)
{
	UIRendererLua::lua_openui(L, uiRenderer);
}

void ScriptHandler::setDebugRenderer(DebugRenderer* debugRenderer)
{
	DebugRendererLua::lua_open_debug_renderer(L, debugRenderer);
}

bool ScriptHandler::runScript(std::string& path)
{
	bool result = luaL_dofile(L, path.c_str()) == LUA_OK;
	if (!result) { LuaH::dumpError(L); }
	return result;
}

void ScriptHandler::runFunction(Entity e, Script& script, const std::string& func)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, script.luaRef);
#ifdef _CONSOLE
	if (luaL_dofile(L, script.path) != LUA_OK)
	{
		LuaH::dumpError(L);
	}
	else
	{
		LuaH::replaceTableFunctions(L, -1, -2);
		lua_pop(L, 1);
	}
#endif

	lua_getfield(L, -1, func.c_str());
	if (lua_type(L, -1) == LUA_TNIL)
	{
		lua_pop(L, 2);
		return;
	}

	Transform& transform = this->sceneHandler->getScene()->getComponent<Transform>(e);
	lua_pushtransform(L, transform);
	lua_setfield(L, -3, "transform");

	lua_pushvalue(L, -2); // self

	// call function
	LUA_ERR_CHECK(L, lua_pcall(L, 1, 0, 0));

	lua_getfield(L, -1, "transform");
	if (!lua_isnil(L, -1))
	{
		transform = lua_totransform(L, -1);
	}
	lua_pop(L, 2);
}

void ScriptHandler::setScriptComponent(Entity entity, std::string& path)
{
	Scene* scene = this->sceneHandler->getScene();

	if (luaL_dofile(L, path.c_str()) != LUA_OK) { LuaH::dumpError(L); }
	else
	{
		lua_pushvalue(L, -1);
		int luaRef = luaL_ref(L, LUA_REGISTRYINDEX);

		lua_pushinteger(L, entity);
		lua_setfield(L, -2, "ID");

		lua_pushstring(L, path.c_str());
		lua_setfield(L, -2, "path");

		Transform& t = scene->getComponent<Transform>(entity);
		lua_pushtransform(L, scene->getComponent<Transform>(entity));
		lua_setfield(L, -2, "transform");

		scene->setComponent<Script>(entity, path.c_str(), luaRef);

		lua_getfield(L, -1, "init");
		if (lua_type(L, -1) == LUA_TNIL) { lua_pop(L, 2); }

		lua_pushvalue(L, -2);
		if (lua_pcall(L, 1, 0, 0) != LUA_OK) { LuaH::dumpError(L); }
		else
		{
			lua_getfield(L, -1, "transform");
			if (!lua_isnil(L, -1))
			{
				scene->setComponent<Transform>(entity, lua_totransform(L, -1));
			}
			lua_pop(L, 1);
		}
		lua_pop(L, 1);
	}
}

void ScriptHandler::runCollisionFunction(Script& script, Entity e1, Entity e2, bool isTrigger, CallbackType type)
{
	std::string func = isTrigger ? "onTrigger" : "onCollision";
	std::string typeStr;
	switch (type)
	{
	case CallbackType::ENTER:
		typeStr = "Enter";
		break;
	case CallbackType::STAY:
		typeStr = "Stay";
		break;
	case CallbackType::EXIT:
		typeStr = "Exit";
		break;
	}

	func += typeStr;
	Transform& transform = this->sceneHandler->getScene()->getComponent<Transform>(e1);

	lua_rawgeti(L, LUA_REGISTRYINDEX, script.luaRef);
#ifdef _CONSOLE
	if (luaL_dofile(L, script.path) != LUA_OK)
	{
		LuaH::dumpError(L);
	}
	else
	{
		LuaH::replaceTableFunctions(L, -1, -2);
		lua_pop(L, 1);
	}
#endif

	lua_getfield(L, -1, func.c_str());
	if (lua_type(L, -1) == LUA_TNIL)
	{
		lua_pop(L, 2);
		return;
	}

	lua_pushtransform(L, transform);
	lua_setfield(L, -3, "transform");

	lua_pushvalue(L, -2); // self
	lua_pushinteger(L, e2); // entity hit

	// call function
	LUA_ERR_CHECK(L, lua_pcall(L, 2, 0, 0));

	lua_getfield(L, -1, "transform");
	if (!lua_isnil(L, -1))
	{
		transform = lua_totransform(L, -1);
	}
	lua_pop(L, 2);
}

void ScriptHandler::updateSystems(std::vector<LuaSystem>& vec)
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

		lua_getfield(L, -2, "update"); // Get new update
		if (!lua_isnil(L, -1)) // Found update
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
			if (returned) { it = vec.erase(it); }
			else { it++; }
		}
		else { it++; }
		lua_pop(L, 3);
	}
}

void ScriptHandler::update()
{
	this->updateScripts();

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

void ScriptHandler::cleanup()
{
	lua_close(L);
}

bool ScriptHandler::getScriptComponentValue(Script& script, int& ret, std::string name)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, script.luaRef);
	lua_getfield(L, -1, name.c_str());

	bool result = lua_isnumber(L, -1);
	if (result) { ret = (int)lua_tonumber(L, -1); }
	lua_pop(L, 2);
	return result;
}

bool ScriptHandler::getScriptComponentValue(Script& script, float& ret, std::string name)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, script.luaRef);
	lua_getfield(L, -1, name.c_str());

	bool result = lua_isnumber(L, -1);
	if (result) { ret = (float)lua_tonumber(L, -1); }
	lua_pop(L, 2);
	return result;
}

bool ScriptHandler::getScriptComponentValue(Script& script, std::string& ret, std::string name)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, script.luaRef);
	lua_getfield(L, -1, name.c_str());

	bool result = lua_isstring(L, -1);
	if (result) { ret = lua_tostring(L, -1); }
	lua_pop(L, 2);
	return result;
}

bool ScriptHandler::getGlobal(int& ret, std::string& name)
{
	lua_getglobal(L, name.c_str());

	bool result = lua_isnumber(L, -1);
	if (result) { ret = (int)lua_tonumber(L, -1); }
	lua_pop(L, 1);
	return result;
}

bool ScriptHandler::getGlobal(float& ret, std::string& name)
{
	lua_getglobal(L, name.c_str());

	bool result = lua_isnumber(L, -1);
	if (result) { ret = (float)lua_tonumber(L, -1); }
	lua_pop(L, 1);
	return result;
}

bool ScriptHandler::getGlobal(std::string& ret, std::string& name)
{
	lua_getglobal(L, name.c_str());

	bool result = lua_isstring(L, -1);
	if (result) { ret = lua_tostring(L, -1); }
	lua_pop(L, 1);
	return result;
}
