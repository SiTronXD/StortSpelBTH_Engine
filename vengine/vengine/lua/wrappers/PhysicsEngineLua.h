#pragma once 
 #include "op_overload.hpp"

#include "../../physics/PhysicsEngine.h"
#include "../LuaPushes.hpp"

class PhysicsEngineLua
{
private:
	inline static const std::vector<std::string> colliderTypes{
		"Sphere",
		"Box",
		"Capsule"
	};

	static int lua_renderDebugShapes(lua_State* L);
	static int lua_raycast(lua_State* L);
	static int lua_testContact(lua_State* L);
public:
	static void lua_openphysics(lua_State* L, PhysicsEngine* physicsEngine);
};

