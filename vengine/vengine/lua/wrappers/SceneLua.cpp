#include "SceneLua.h"
#include "../../application/SceneHandler.hpp"
#include "../../application/Time.hpp"
#include "../../components/Behaviour.hpp"
#include "../../components/MeshComponent.hpp"
#include "../LuaPushes.hpp"


int SceneLua::lua_createSystem(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
	int type = (int)lua_tointeger(L, 1);

	return 0;
}

int SceneLua::lua_setScene(lua_State* L)
{
	SceneHandler* sceneHandler = (SceneHandler*)lua_touserdata(L, lua_upvalueindex(1));
	std::string path = lua_tostring(L, 1);
	sceneHandler->setScene(path);

	return 0;
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
	SceneHandler* sh = (SceneHandler*)lua_touserdata(L, lua_upvalueindex(1));
	Scene* scene = sh->getScene();
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
		break;
	case CompType::MESH:
		// Get from resource manager
		scene->setComponent<MeshComponent>(entity, /*lua_tostring(L, 3)*/0);
		break;
	case CompType::BEHAVIOUR:
		path = lua_tostring(L, 3);
		if (luaL_dofile(L, path.c_str()) != LUA_OK)
			LuaH::dumpError(L);
		else
		{
			lua_pushvalue(L, -1);
			int luaRef = luaL_ref(L, LUA_REGISTRYINDEX);

			lua_pushinteger(L, entity);
			lua_setfield(L, -2, "ID");

			lua_pushstring(L, path.c_str());
			lua_setfield(L, -2, "path");

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
