#include "NetworkSceneLua.h"
#include "../../application/Time.hpp"

int NetworkSceneLua::lua_createSystem(lua_State* L)
{
	NetworkScene* scene = ((NetworkScene*)lua_touserdata(L, lua_upvalueindex(1)));

	if (lua_isstring(L, 1))  // Lua System
	{
		std::string str = lua_tostring(L, 1);
		scene->createSystem(str);
	}
	else if (lua_isnumber(L, 1))  // C++ System
	{
		int type = lua_tonumber(L, 1);
	}

	return 0;
}

int NetworkSceneLua::lua_iterateView(lua_State* L)
{
	NetworkScene* scene = ((NetworkScene*)lua_touserdata(L, lua_upvalueindex(1)));

	// Sanity check
	if (!lua_istable(L, 1) || !lua_isfunction(L, 2))
	{
		std::cout << "Error: iterate view arguments" << std::endl;
		return 0;
	}

	std::string scriptPath;
	std::vector<CompType> compTypes;

	lua_pushvalue(L, 1);
	lua_pushnil(L);
	while (lua_next(L, -2))
	{
		lua_pushvalue(L, -2);

		if (lua_isstring(L, -2) && !scriptPath.size())
		{
			scriptPath = lua_tostring(L, -2);
			compTypes.push_back(CompType::SCRIPT);
		}
		else if (lua_isnumber(L, -2))
		{
			compTypes.push_back((CompType)lua_tointeger(L, -2));
		}

		lua_pop(L, 2);
	}
	lua_pop(L, 1);

	std::vector<int> entitiesToIterate;

	const entt::entity* data = scene->getSceneReg().data();
	const int size = scene->getEntityCount();
	for (int i = 0; i < size; i++)
	{
		int entity = (int)data[i];
		bool addToList = std::find(compTypes.begin(), compTypes.end(), CompType::TRANSFORM) != compTypes.end() && scene->hasComponents<Transform>(entity) &&
		                 std::find(compTypes.begin(), compTypes.end(), CompType::MESH) != compTypes.end() && scene->hasComponents<MeshComponent>(entity) &&
		                 std::find(compTypes.begin(), compTypes.end(), CompType::SCRIPT) != compTypes.end() && scene->hasComponents<Script>(entity);

		if (scriptPath.size() && addToList)
		{
			Script& script = scene->getComponent<Script>(entity);
			addToList = addToList && script.path == scriptPath;
		}
		if (addToList)
		{
			entitiesToIterate.push_back(entity);
		}
	}

	bool useAddtionalTable = lua_istable(L, 3);
	for (const auto& entity : entitiesToIterate)
	{
		lua_pushvalue(L, 2);  // Function
		if (useAddtionalTable)
		{
			lua_pushvalue(L, 3);
		}  // Own table
		for (const auto& type : compTypes)
		{
			switch (type)
			{
				case CompType::TRANSFORM:
					lua_pushtransform(L, scene->getComponent<Transform>(entity));
					break;
				case CompType::MESH:
					lua_pushmesh(L, scene->getComponent<MeshComponent>(entity));
					break;
				case CompType::SCRIPT:
					lua_rawgeti(L, LUA_REGISTRYINDEX, scene->getComponent<Script>(entity).luaRef);
					break;
				default:
					break;
			}
		}

		LUA_ERR_CHECK(L, lua_pcall(L, (int)compTypes.size() + (useAddtionalTable), 0, 0));
	}

	return 0;
}

int NetworkSceneLua::lua_createPrefab(lua_State* L)
{
	NetworkScene* scene = (NetworkScene*)lua_touserdata(L, lua_upvalueindex(1));

	if (lua_isstring(L, 1))  // Prefab file
	{
		std::string path = lua_tostring(L, 1);
		if (luaL_dofile(L, path.c_str()) != LUA_OK)
		{
			LuaH::dumpError(L);
			return 0;
		}
	}

	if (!lua_istable(L, -1))
	{
		return 0;
	}

	int entity = scene->createEntity();

	lua_getfield(L, -1, "Transform");
	if (!lua_isnil(L, -1))
	{
		scene->setComponent<Transform>(entity, lua_totransform(L, -1));
	}
	lua_pop(L, 1);

	lua_getfield(L, -1, "Script");
	if (lua_isstring(L, -1))
	{
		scene->setScriptComponent(entity, lua_tostring(L, -1));
	}
	lua_pop(L, 1);

	lua_pushnumber(L, entity);
	return 1;
}

int NetworkSceneLua::lua_getEntityCount(lua_State* L)
{
	NetworkScene* scene = ((NetworkScene*)lua_touserdata(L, lua_upvalueindex(1)));
	lua_pushnumber(L, scene->getEntityCount());
	return 1;
}

int NetworkSceneLua::lua_entityValid(lua_State* L)
{
	NetworkScene* scene = ((NetworkScene*)lua_touserdata(L, lua_upvalueindex(1)));
	lua_pushboolean(L, scene->entityValid((int)lua_tointeger(L, 1)));
	return 1;
}

int NetworkSceneLua::lua_createEntity(lua_State* L)
{
	NetworkScene* scene = ((NetworkScene*)lua_touserdata(L, lua_upvalueindex(1)));
	lua_pushnumber(L, scene->createEntity());
	return 1;
}

int NetworkSceneLua::lua_removeEntity(lua_State* L)
{
	NetworkScene* scene = ((NetworkScene*)lua_touserdata(L, lua_upvalueindex(1)));
	lua_pushboolean(L, scene->removeEntity((int)lua_tointeger(L, 1)));
	return 1;
}

int NetworkSceneLua::lua_hasComponent(lua_State* L)
{
	NetworkScene* scene = ((NetworkScene*)lua_touserdata(L, lua_upvalueindex(1)));
	int entity = (int)lua_tointeger(L, 1);
	int type = (int)lua_tointeger(L, 2);
	bool hasComp = false;

	switch ((CompType)type)
	{
		case CompType::TRANSFORM:
			hasComp = scene->hasComponents<Transform>(entity);
			break;
		case CompType::MESH:
			hasComp = scene->hasComponents<MeshComponent>(entity);
			break;
		case CompType::SCRIPT:
			hasComp = scene->hasComponents<Script>(entity);
			break;
		case CompType::CAMERA:
			hasComp = scene->hasComponents<Camera>(entity);
			break;
		default:
			break;
	}

	lua_pushboolean(L, hasComp);
	return 1;
}

int NetworkSceneLua::lua_getComponent(lua_State* L)
{
	NetworkScene* scene = ((NetworkScene*)lua_touserdata(L, lua_upvalueindex(1)));
	int entity = (int)lua_tointeger(L, 1);
	int type = (int)lua_tointeger(L, 2);

	if ((CompType)type == CompType::TRANSFORM && scene->hasComponents<Transform>(entity))
	{
		lua_pushtransform(L, scene->getComponent<Transform>(entity));
	}
	else if ((CompType)type == CompType::MESH && scene->hasComponents<MeshComponent>(entity))
	{
		lua_pushmesh(L, scene->getComponent<MeshComponent>(entity));
	}
	else if ((CompType)type == CompType::SCRIPT && scene->hasComponents<Script>(entity))
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, scene->getComponent<Script>(entity).luaRef);
	}
	else
	{
		lua_pushnil(L);
	}
	return 1;
}

int NetworkSceneLua::lua_setComponent(lua_State* L)
{
	NetworkScene* scene = (NetworkScene*)lua_touserdata(L, lua_upvalueindex(1));

	int entity = (int)lua_tointeger(L, 1);
	int type = (int)lua_tointeger(L, 2);
	std::string path;

	switch ((CompType)type)
	{
		case CompType::TRANSFORM:
			scene->setComponent<Transform>(entity, lua_totransform(L, 3));
			break;
		case CompType::SCRIPT:
			if (lua_isstring(L, 3))
			{
				scene->setScriptComponent(entity, lua_tostring(L, 3));
			}
			break;
		default:
			break;
	}

	return 0;
}

int NetworkSceneLua::lua_removeComponent(lua_State* L)
{
	NetworkScene* scene = ((NetworkScene*)lua_touserdata(L, lua_upvalueindex(1)));
	int entity = (int)lua_tointeger(L, 1);
	int type = (int)lua_tointeger(L, 2);

	switch ((CompType)type)
	{
		case CompType::SCRIPT:
			scene->removeComponent<Script>(entity);
			break;
		default:
			break;
	}

	return 0;
}

int NetworkSceneLua::lua_getPlayer(lua_State* L)
{
	NetworkScene* scene = ((NetworkScene*)lua_touserdata(L, lua_upvalueindex(1)));
	lua_pushnumber(L, scene->getPlayer((int)lua_tointeger(L, 1)));
	return 1;
}

int NetworkSceneLua::lua_getPlayerCount(lua_State* L)
{
	NetworkScene* scene = ((NetworkScene*)lua_touserdata(L, lua_upvalueindex(1)));
	lua_pushnumber(L, scene->getPlayerSize());
	return 1;
}

int NetworkSceneLua::lua_addEvent(lua_State* L) {
	NetworkScene* scene = ((NetworkScene*)lua_touserdata(L, lua_upvalueindex(1)));
	std::vector<int> ints;
	std::vector<float> floats;
	ints.push_back(lua_tointeger(L, 1));
	//check if that was all

	//if not check if its a table
	//if its a table get that and return

	//else its a number 
	int i = 2;
	bool done = false;
	while (lua_gettop(L) > 0 && !done)
	{
		if (!lua_istable(L, i))
		{
			ints.push_back(lua_tointeger(L, i));
		}
		else
		{
			//go trough table
			lua_pushnil(L);
			while (lua_next(L, -2))
			{
				lua_pushvalue(L, -2);
				const char* key = lua_tostring(L, -1);

				floats.push_back((int)lua_tonumber(L, -2));

				lua_pop(L, 2);
			}
			lua_pop(L, 1);
			done = true;
		}
		i++;
	}
	scene->addEvent(ints, floats);
	lua_pop(L, lua_gettop(L));
	return 0;
}

void NetworkSceneLua::lua_openscene(lua_State* L, NetworkScene* scene)
{
	lua_newtable(L);

	luaL_Reg methods[] = {
	    {"createSystem", lua_createSystem},
	    {"iterateView", lua_iterateView},
	    {"createPrefab", lua_createPrefab},
	    {"getEntityCount", lua_getEntityCount},
	    {"entityValid", lua_entityValid},
	    {"createEntity", lua_createEntity},
	    {"removeEntity", lua_removeEntity},
	    {"hasComponent", lua_hasComponent},
	    {"getComponent", lua_getComponent},
	    {"setComponent", lua_setComponent},
	    {"removeComponent", lua_removeComponent},
	    {"getPlayer", lua_getPlayer},
	    {"getPlayerCount", lua_getPlayerCount},
	    {"addEvent", lua_addEvent},
	    {NULL, NULL}};

	lua_pushlightuserdata(L, scene);
	luaL_setfuncs(L, methods, 1);
	lua_setglobal(L, "scene");

	lua_newtable(L);
	for (size_t i = 0; i < systemTypes.size(); i++)
	{
		lua_pushnumber(L, (int)i);
		lua_setfield(L, -2, systemTypes[i].c_str());
	}
	lua_setglobal(L, "SystemType");

	lua_newtable(L);
	for (size_t i = 0; i < compTypes.size(); i++)
	{
		lua_pushnumber(L, (int)i);
		lua_setfield(L, -2, compTypes[i].c_str());
	}
	lua_setglobal(L, "CompType");
}
