#pragma once

#include <vector>
#include <string>
#include "../dev/LuaHelper.hpp"
#include "../../application/SceneHandler.hpp"
#include "../../components/Script.hpp"
#include "../../network/NetworkHandler.h"
#include "../../components/MeshComponent.hpp"
#include "../LuaPushes.hpp"

class SceneLua
{
private:
	// COUNT: Getting the number of Components
	enum class CompType { TRANSFORM, MESH, SCRIPT, CAMERA, COUNT };
	inline static const std::vector<std::string> compTypes {
		"Transform",
		"Mesh",
		"Script",
		"Camera"
	};

	// COUNT: Getting the number of Systems
	enum class SystemType { COUNT };
	inline static const std::vector<std::string> systemTypes {
	};

	static int lua_createSystem(lua_State* L);
	static int lua_setScene(lua_State* L);
	static int lua_iterateView(lua_State* L);
	static int lua_createPrefab(lua_State* L);

	static int lua_getMainCamera(lua_State* L);
	static int lua_setMainCamera(lua_State* L);

	static int lua_getEntityCount(lua_State* L);
	static int lua_entityValid(lua_State* L);
	static int lua_createEntity(lua_State* L);
	static int lua_removeEntity(lua_State* L);

	static int lua_hasComponent(lua_State* L);
	static int lua_getComponent(lua_State* L);
	static int lua_setComponent(lua_State* L);
	static int lua_removeComponent(lua_State* L);

	//network
	static int lua_sendPolygons(lua_State* L);
	static int lua_isServer(lua_State* L);
public:
	static void lua_openscene(lua_State* L, SceneHandler* sceneHandler);
	static void lua_openNetworkScene(lua_State* L, NetworkHandler* networkHandler);
};