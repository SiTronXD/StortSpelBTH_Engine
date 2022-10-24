#pragma once
#pragma once

#include <string>
#include <vector>
#include "NetworkScene.h"
#include "../../components/MeshComponent.hpp"
#include "../../components/Script.hpp"
#include "../../lua/LuaPushes.hpp"
#include "../../lua/dev/LuaHelper.hpp"

class NetworkSceneLua
{
   private:
	NetworkScene* scene;

	// COUNT: Getting the number of Components
	enum class CompType
	{
		TRANSFORM,
		MESH,
		SCRIPT,
		CAMERA,
		COUNT
	};
	inline static const std::vector<std::string> compTypes{
	    "Transform",
	    "Mesh",
	    "Script",
	    "Camera"};

	// COUNT: Getting the number of Systems
	enum class SystemType
	{
		COUNT
	};
	inline static const std::vector<std::string> systemTypes{};

	static int lua_createSystem(lua_State* L);
	static int lua_iterateView(lua_State* L);
	static int lua_createPrefab(lua_State* L);

	static int lua_getEntityCount(lua_State* L);
	static int lua_entityValid(lua_State* L);
	static int lua_createEntity(lua_State* L);
	static int lua_removeEntity(lua_State* L);

	static int lua_hasComponent(lua_State* L);
	static int lua_getComponent(lua_State* L);
	static int lua_setComponent(lua_State* L);
	static int lua_removeComponent(lua_State* L);
	static int lua_getPlayer(lua_State* L);
	static int lua_getPlayerCount(lua_State* L);
	static int lua_addEvent(lua_State* L);

   public:
	static void lua_openscene(lua_State* L, NetworkScene* scene);
};