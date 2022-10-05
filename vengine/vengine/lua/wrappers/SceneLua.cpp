#include "SceneLua.h"
#include "../../application/Time.hpp"


int SceneLua::lua_createSystem(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();

	if (lua_isstring(L, 1)) // Lua System
	{ 
		std::string str = lua_tostring(L, 1);
		scene->createSystem(str);
	}
	else if (lua_isnumber(L, 1)) // C++ System
	{ 
		int type = lua_tonumber(L, 1);
	}

	return 0;
}

int SceneLua::lua_setScene(lua_State* L)
{
	SceneHandler* sceneHandler = (SceneHandler*)lua_touserdata(L, lua_upvalueindex(1));
	std::string path = lua_tostring(L, 1);
	sceneHandler->setScene(new Scene(), path);

	return 0;
}

int SceneLua::lua_iterateView(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();

	// Sanity check
	if (!lua_istable(L, 1) || !lua_istable(L, 2) || lua_isnil(L, 3))
	{
		std::cout << "Error: iterate view arguments" << std::endl;
		return 0;
	}

	std::string behaviourPath;
	std::vector<CompType> compTypes;

	lua_pushvalue(L, 2);
	lua_pushnil(L);
	while (lua_next(L, -2))
	{
		lua_pushvalue(L, -2);

		if (lua_isstring(L, -2) && !behaviourPath.size())
		{
			behaviourPath = lua_tostring(L, -2);
			compTypes.push_back(CompType::BEHAVIOUR);
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
		bool addToList =
			std::find(compTypes.begin(), compTypes.end(), CompType::TRANSFORM) != compTypes.end() && scene->hasComponents<Transform>(entity) &&
			std::find(compTypes.begin(), compTypes.end(), CompType::MESH) != compTypes.end() && scene->hasComponents<MeshComponent>(entity) &&
			std::find(compTypes.begin(), compTypes.end(), CompType::BEHAVIOUR) != compTypes.end() && scene->hasComponents<Behaviour>(entity);
			

		if (behaviourPath.size() && addToList)
		{
			Behaviour& behaviour = scene->getComponent<Behaviour>(entity);
			addToList = addToList && behaviour.path == behaviourPath;
		}
		if (addToList) { entitiesToIterate.push_back(entity); }
	}

	for (const auto& entity : entitiesToIterate)
	{
		lua_pushvalue(L, -1); // Function
		lua_pushvalue(L, 1); // Own table
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
			case CompType::BEHAVIOUR:
				lua_rawgeti(L, LUA_REGISTRYINDEX, scene->getComponent<Behaviour>(entity).luaRef);
				break;
			default:
				break;
			}
		}

		LUA_ERR_CHECK(L, lua_pcall(L, (int)compTypes.size() + 1, 0, 0));
	}

	return 0;
}

int SceneLua::lua_createPrefab(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();

	if (lua_isstring(L, 1)) // Prefab file
	{
		std::string path = lua_tostring(L, 1);
		if (luaL_dofile(L, path.c_str()) != LUA_OK)
		{
			LuaH::dumpError(L);
			return 0;
		}
	}

	if (!lua_istable(L, -1)) { return 0; }

	int entity = scene->createEntity();

	lua_getfield(L, -1, "Transform");
	if (!lua_isnil(L, -1))
	{
		scene->setComponent<Transform>(entity, lua_totransform(L, -1));
	}
	lua_pop(L, 1);

	lua_getfield(L, -1, "Mesh");
	if (!lua_isnil(L, -1))
	{
		scene->setComponent<MeshComponent>(entity, (int)lua_tointeger(L, -1));
	}
	lua_pop(L, 1);

	lua_getfield(L, -1, "Camera");
	if (!lua_isnil(L, -1))
	{
		scene->setComponent<Camera>(entity, (float)lua_tonumber(L, -1));
	}
	lua_pop(L, 1);

	lua_getfield(L, -1, "Behaviour");
	if (!lua_isnil(L, -1))
	{
		std::string path = lua_tostring(L, -1);
		if (luaL_dofile(L, path.c_str()) != LUA_OK)
		{ LuaH::dumpError(L); }
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

			scene->setComponent<Behaviour>(entity, path.c_str(), luaRef);

			lua_getfield(L, -1, "init");
			if (lua_type(L, -1) != LUA_TNIL)
			{
				lua_pushvalue(L, -2);
				LUA_ERR_CHECK(L, lua_pcall(L, 1, 0, 0));
			}
		}
	}
	lua_pop(L, 1);

	lua_pushnumber(L, entity);
	return 1;
}

int SceneLua::lua_getMainCamera(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
	lua_pushinteger(L, scene->getMainCameraID());

	return 1;
}

int SceneLua::lua_setMainCamera(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
	int entity = (int)lua_tointeger(L, 1);
	scene->setMainCamera(entity);

	return 0;
}

int SceneLua::lua_getEntityCount(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
	lua_pushnumber(L, scene->getEntityCount());
	return 1;
}

int SceneLua::lua_entityValid(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
	lua_pushboolean(L, scene->entityValid((int)lua_tointeger(L, 1)));
	return 1;
}

int SceneLua::lua_createEntity(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
	lua_pushnumber(L, scene->createEntity());
	return 1;
}

int SceneLua::lua_removeEntity(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
	lua_pushboolean(L, scene->removeEntity((int)lua_tointeger(L, 1)));
	return 1;
}

int SceneLua::lua_hasComponent(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
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
	case CompType::BEHAVIOUR:
		hasComp = scene->hasComponents<Behaviour>(entity);
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

int SceneLua::lua_getComponent(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
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
	else if ((CompType)type == CompType::BEHAVIOUR && scene->hasComponents<Behaviour>(entity))
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, scene->getComponent<Behaviour>(entity).luaRef);
	}
	else { lua_pushnil(L); }
	return 1;
}

int SceneLua::lua_setComponent(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
	int entity = (int)lua_tointeger(L, 1);
	int type = (int)lua_tointeger(L, 2);
	std::string path;

	switch ((CompType)type)
	{
	case CompType::TRANSFORM:
		scene->setComponent<Transform>(entity, lua_totransform(L, 3));
		if (scene->hasComponents<Behaviour>(entity))
		{
			const Behaviour& script = scene->getComponent<Behaviour>(entity);

			lua_rawgeti(L, LUA_REGISTRYINDEX, script.luaRef);
			lua_pushtransform(L, scene->getComponent<Transform>(entity));
			lua_setfield(L, -2, "transform");

			lua_pop(L, 1);
		}
		break;
	case CompType::MESH:
		// Get from resource manager
		scene->setComponent<MeshComponent>(entity, /*lua_tostring(L, 3)*/0);
		break;
	case CompType::BEHAVIOUR:
		path = lua_tostring(L, 3);
		if (luaL_dofile(L, path.c_str()) != LUA_OK)
		{ LuaH::dumpError(L); }
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

			scene->setComponent<Behaviour>(entity, path.c_str(), luaRef);

			lua_getfield(L, -1, "init");
			if (lua_type(L, -1) != LUA_TNIL)
			{
				lua_pushvalue(L, -2);
				LUA_ERR_CHECK(L, lua_pcall(L, 1, 0, 0));
			}
		}
		break;
	case CompType::CAMERA:
		scene->setComponent<Camera>(entity, 1.0f);
		break;
	default:
		break;
	}

	return 0;
}

int SceneLua::lua_removeComponent(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
	int entity = (int)lua_tointeger(L, 1);
	int type = (int)lua_tointeger(L, 2);

	switch ((CompType)type)
	{
	case CompType::MESH:
		scene->removeComponent<MeshComponent>(entity);
		break;
	case CompType::BEHAVIOUR:
		scene->removeComponent<Behaviour>(entity);
		break;
	case CompType::CAMERA:
		scene->removeComponent<Camera>(entity);
		break;
	default:
		break;
	}

	return 0;
}

void SceneLua::lua_openscene(lua_State* L, SceneHandler* sceneHandler)
{
	lua_newtable(L);

	luaL_Reg methods[] = {
		{ "createSystem", lua_createSystem },
		{ "setScene", lua_setScene },
		{ "iterateView", lua_iterateView },
		{ "createPrefab", lua_createPrefab },
		{ "getMainCamera", lua_getMainCamera },
		{ "setMainCamera", lua_setMainCamera },
		{ "getEntityCount", lua_getEntityCount },
		{ "entityValid", lua_entityValid },
		{ "createEntity", lua_createEntity },
		{ "removeEntity", lua_removeEntity },
		{ "hasComponent", lua_hasComponent },
		{ "getComponent", lua_getComponent },
		{ "setComponent", lua_setComponent },
		{ "removeComponent", lua_removeComponent },
		{ NULL , NULL }
	};

	lua_pushlightuserdata(L, sceneHandler);
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
