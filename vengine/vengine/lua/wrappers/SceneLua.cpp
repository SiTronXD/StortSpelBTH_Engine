#include "pch.h"
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

	std::vector<Entity> entitiesToIterate;

	const entt::entity* data = scene->getSceneReg().data();
	const int size = scene->getEntityCount();
	for (int i = 0; i < size; i++)
	{
		int entity = (int)data[i];
		bool addToList =
			std::find(compTypes.begin(), compTypes.end(), CompType::TRANSFORM) != compTypes.end() && scene->hasComponents<Transform>(entity) &&
			std::find(compTypes.begin(), compTypes.end(), CompType::MESH) != compTypes.end() && scene->hasComponents<MeshComponent>(entity) &&
			std::find(compTypes.begin(), compTypes.end(), CompType::CAMERA) != compTypes.end() && scene->hasComponents<Camera>(entity) &&
			std::find(compTypes.begin(), compTypes.end(), CompType::SCRIPT) != compTypes.end() && scene->hasComponents<Script>(entity) &&
			std::find(compTypes.begin(), compTypes.end(), CompType::COLLIDER) != compTypes.end() && scene->hasComponents<Collider>(entity) &&
			std::find(compTypes.begin(), compTypes.end(), CompType::RIGIDBODY) != compTypes.end() && scene->hasComponents<Rigidbody>(entity) &&
			std::find(compTypes.begin(), compTypes.end(), CompType::ANIMATION) != compTypes.end() && scene->hasComponents<AnimationComponent>(entity) &&
			std::find(compTypes.begin(), compTypes.end(), CompType::UIAREA) != compTypes.end() && scene->hasComponents<UIArea>(entity);

		if (scriptPath.size() && addToList)
		{
			Script& script = scene->getComponent<Script>(entity);
			addToList = addToList && script.path == scriptPath;
		}
		if (addToList) { entitiesToIterate.push_back(entity); }
	}

	bool useAddtionalTable = lua_istable(L, 3);
	for (const auto& entity : entitiesToIterate)
	{
		lua_pushvalue(L, 2); // Function
		if(useAddtionalTable) { lua_pushvalue(L, 3); } // Own table
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
			case CompType::CAMERA:
				lua_pushcamera(L, scene->getComponent<Camera>(entity));
				break;
			case CompType::SCRIPT:
				lua_rawgeti(L, LUA_REGISTRYINDEX, scene->getComponent<Script>(entity).luaRef);
				break;
			case CompType::COLLIDER:
				lua_pushcollider(L, scene->getComponent<Collider>(entity));
				break;
			case CompType::RIGIDBODY:
				lua_pushrigidbody(L, scene->getComponent<Rigidbody>(entity));
				break;
			case CompType::ANIMATION:
				lua_pushanimation(L, scene->getComponent<AnimationComponent>(entity));
				break;
			case CompType::UIAREA:
				lua_pushuiarea(L, scene->getComponent<UIArea>(entity));
				break;
			default:
				break;
			}
		}

		LUA_ERR_CHECK(L, lua_pcall(L, (int)compTypes.size() + (useAddtionalTable), 0, 0));
	}

	return 0;
}

int SceneLua::lua_createPrefab(lua_State* L)
{
	SceneHandler* sceneHandler = (SceneHandler*)lua_touserdata(L, lua_upvalueindex(1));
	Scene* scene = sceneHandler->getScene();

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
	if (lua_isnumber(L, -1))
	{
		scene->setComponent<MeshComponent>(entity, (int)lua_tointeger(L, -1));
	}
	else if (lua_isstring(L, -1))
	{
		scene->setComponent<MeshComponent>(entity, (int)sceneHandler->getResourceManager()->addMesh(lua_tostring(L, -1)));
	}
	lua_pop(L, 1);

	lua_getfield(L, -1, "Script");
	if (lua_isstring(L, -1))
	{
		scene->setScriptComponent(entity, lua_tostring(L, -1));
	}
	lua_pop(L, 1);

	lua_getfield(L, -1, "Camera");
	if (lua_isstring(L, -1))
	{
		scene->setComponent<Camera>(entity, (float)lua_tonumber(L, -1));
	}
	lua_pop(L, 1);

	lua_getfield(L, -1, "Collider");
	if (!lua_isnil(L, -1))
	{
		scene->setComponent<Collider>(entity, lua_tocollider(L, -1));
	}
	lua_pop(L, 1);

	lua_getfield(L, -1, "Rigidbody");
	if (!lua_isnil(L, -1))
	{
		scene->setComponent<Rigidbody>(entity, lua_torigidbody(L, -1));
	}
	lua_pop(L, 1);

	lua_getfield(L, -1, "Animation");
	if (!lua_isnil(L, -1))
	{
		if (scene->hasComponents<AnimationComponent>(entity))
		{
			StorageBufferID boneID = scene->getComponent<AnimationComponent>(entity).boneTransformsID;
			scene->setComponent<AnimationComponent>(entity, lua_toanimation(L, -1, boneID));
		}
		else
		{
			scene->setComponent<AnimationComponent>(entity, lua_toanimation(L, -1, 0));
		}
	}
	lua_pop(L, 1);

	lua_getfield(L, -1, "UIArea");
	if (!lua_isnil(L, -1))
	{
		scene->setComponent<UIArea>(entity, lua_touiarea(L, -1));
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
	case CompType::SCRIPT:
		hasComp = scene->hasComponents<Script>(entity);
		break;
	case CompType::CAMERA:
		hasComp = scene->hasComponents<Camera>(entity);
		break;
	case CompType::COLLIDER:
		hasComp = scene->hasComponents<Collider>(entity);
		break;
	case CompType::RIGIDBODY:
		hasComp = scene->hasComponents<Rigidbody>(entity);
		break;
	case CompType::ANIMATION:
		hasComp = scene->hasComponents<AnimationComponent>(entity);
		break;
	case CompType::UIAREA:
		hasComp = scene->hasComponents<UIArea>(entity);
		break;
	case CompType::AUDIOSOURCE:
		hasComp = scene->hasComponents<AudioSource>(entity);
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
	else if ((CompType)type == CompType::SCRIPT && scene->hasComponents<Script>(entity))
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, scene->getComponent<Script>(entity).luaRef);
	}
	else if ((CompType)type == CompType::CAMERA && scene->hasComponents<Camera>(entity))
	{
		lua_pushcamera(L, scene->getComponent<Camera>(entity));
	}
	else if ((CompType)type == CompType::COLLIDER && scene->hasComponents<Collider>(entity))
	{
		lua_pushcollider(L, scene->getComponent<Collider>(entity));
	}
	else if ((CompType)type == CompType::RIGIDBODY && scene->hasComponents<Rigidbody>(entity))
	{
		lua_pushrigidbody(L, scene->getComponent<Rigidbody>(entity));
	}
	else if ((CompType)type == CompType::ANIMATION && scene->hasComponents<AnimationComponent>(entity))
	{
		lua_pushanimation(L, scene->getComponent<AnimationComponent>(entity));
	}
	else if ((CompType)type == CompType::UIAREA && scene->hasComponents<UIArea>(entity))
	{
		lua_pushuiarea(L, scene->getComponent<UIArea>(entity));
	}
	else { lua_pushnil(L); }
	return 1;
}

int SceneLua::lua_setComponent(lua_State* L)
{
	SceneHandler* sceneHandler = (SceneHandler*)lua_touserdata(L, lua_upvalueindex(1));
	Scene* scene = sceneHandler->getScene();

	int entity = (int)lua_tointeger(L, 1);
	int type = (int)lua_tointeger(L, 2);
	std::string path;

	StorageBufferID boneID = 0;
	bool assigned = false;

	switch ((CompType)type)
	{
	case CompType::TRANSFORM:
		scene->setComponent<Transform>(entity, lua_totransform(L, 3));
		break;
	case CompType::MESH:
		if (lua_isnumber(L, -1))
		{
			scene->setComponent<MeshComponent>(entity, (int)lua_tointeger(L, 3));
		}
		else if (lua_isstring(L, -1))
		{
			scene->setComponent<MeshComponent>(entity, (int)sceneHandler->getResourceManager()->addMesh(lua_tostring(L, 3)));
		}
		break;
	case CompType::SCRIPT:
		if(lua_isstring(L, 3)) { scene->setScriptComponent(entity, lua_tostring(L, 3)); }
		break;
	case CompType::CAMERA:
		scene->setComponent<Camera>(entity, lua_tocamera(L, 3));
		break;
	case CompType::COLLIDER:
		scene->setComponent<Collider>(entity, lua_tocollider(L, 3));
		break;
	case CompType::RIGIDBODY:
		if (scene->hasComponents<Rigidbody>(entity)) { assigned = scene->getComponent<Rigidbody>(entity).assigned; }
		scene->setComponent<Rigidbody>(entity, lua_torigidbody(L, 3, assigned));
		break;
	case CompType::ANIMATION:
		if (scene->hasComponents<AnimationComponent>(entity)) 
		{ 
			boneID = scene->getComponent<AnimationComponent>(entity).boneTransformsID;
		}
		scene->setComponent<AnimationComponent>(entity, lua_toanimation(L, 3, boneID));
		break;
	case CompType::UIAREA:
		scene->setComponent<UIArea>(entity, lua_touiarea(L, 3));
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
	case CompType::SCRIPT:
		scene->removeComponent<Script>(entity);
		break;
	case CompType::CAMERA:
		scene->removeComponent<Camera>(entity);
		break;
	case CompType::COLLIDER:
		scene->removeComponent<Collider>(entity);
		break;
	case CompType::RIGIDBODY:
		scene->removeComponent<Rigidbody>(entity);
		break;
	case CompType::ANIMATION:
		scene->removeComponent<AnimationComponent>(entity);
		break;
	case CompType::UIAREA:
		scene->removeComponent<UIArea>(entity);
		break;
	case CompType::AUDIOSOURCE:
		scene->removeComponent<AudioSource>(entity);
		break;
	default:
		break;
	}

	return 0;
}

int SceneLua::lua_setActive(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
	int entity = (int)lua_tointeger(L, 1);
	scene->setActive(entity);

	return 0;
}

int SceneLua::lua_setInactive(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
	int entity = (int)lua_tointeger(L, 1);
	scene->setInactive(entity);

	return 0;
}

int SceneLua::lua_isActive(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
	int entity = (int)lua_tointeger(L, 1);
	lua_pushboolean(L, scene->isActive(entity));

	return 1;
}

int SceneLua::lua_setAnimation(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
	if (!lua_isnumber(L, 1) || !lua_isstring(L, 2)) { return 0; }
	
	Entity entity = (Entity)lua_tointeger(L, 1);
	scene->setAnimation(entity, lua_tostring(L, 2), lua_tostring(L, 3));

	return 0;
}

int SceneLua::lua_blendToAnimation(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
	if (!lua_isnumber(L, 1) || !lua_isstring(L, 2) || !lua_isstring(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5)) { return 0; }
	
	scene->blendToAnimation((Entity)lua_tointeger(L, 1), lua_tostring(L, 2), lua_tostring(L, 3), (float)lua_tonumber(L, 4), (float)lua_tonumber(L, 5));

	return 0;
}

int SceneLua::lua_syncedBlendToAnimation(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
	if (!lua_isnumber(L, 1) || !lua_isstring(L, 2) || !lua_isstring(L, 3) || !lua_isnumber(L, 4)) { return 0; }

	scene->syncedBlendToAnimation((Entity)lua_tointeger(L, 1), lua_tostring(L, 2), lua_tostring(L, 3), (float)lua_tonumber(L, 4));

	return 0;
}

int SceneLua::lua_getAnimationSlot(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
	Entity entity = (Entity)lua_tointeger(L, 1);
	const char* slotName = lua_tostring(L, 2);

	lua_pushanimationslot(L, scene->getAnimationSlot(entity, slotName));

	return 1;
}

int SceneLua::lua_setAnimationSlot(lua_State* L)
{
	Scene* scene = ((SceneHandler*)lua_touserdata(L, lua_upvalueindex(1)))->getScene();
	Entity entity = (Entity)lua_tointeger(L, 1);
	const char* slotName = lua_tostring(L, 2);

	AnimationSlot& slot = scene->getAnimationSlot(entity, slotName);
	slot.animationIndex = (uint32_t)lua_tointeger(L, 3);
	slot.timer = (float)lua_tonumber(L, 4);
	slot.timeScale = (float)lua_tonumber(L, 5);

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
		{ "setActive", lua_setActive },
		{ "setInactive", lua_setInactive },
		{ "isActive", lua_isActive },
		{ "setAnimation", lua_setAnimation },
		{ "getAnimationSlot", lua_getAnimationSlot },
		{ "setAnimationSlot", lua_setAnimationSlot },
		{ "blendToAnimation", lua_blendToAnimation },
		{ "syncedBlendToAnimation", lua_syncedBlendToAnimation },
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

