#include "pch.h"
#include "PhysicsEngineLua.h"

int PhysicsEngineLua::lua_renderDebugShapes(lua_State* L)
{
	PhysicsEngine* physicsEngine = (PhysicsEngine*)lua_touserdata(L, lua_upvalueindex(1));

	if (lua_isboolean(L, 1)) { physicsEngine->renderDebugShapes(lua_toboolean(L, 1)); }

    return 0;
}

int PhysicsEngineLua::lua_raycast(lua_State* L)
{
	PhysicsEngine* physicsEngine = (PhysicsEngine*)lua_touserdata(L, lua_upvalueindex(1));
	if (lua_isvector(L, 1) && lua_isvector(L, 2))
	{
		float dist = lua_isnumber(L, 3) ? (float)lua_tonumber(L, 3) : 100.0f;
		RayPayload payload = physicsEngine->raycast({ lua_tovector(L, 1), lua_tovector(L, 2) }, dist);

		if (payload.hit)
		{
			lua_newtable(L);

			lua_pushnumber(L, payload.entity);
			lua_setfield(L, -2, "entity");

			lua_pushvector(L, payload.hitPoint);
			lua_setfield(L, -2, "hitPoint");

			lua_pushvector(L, payload.hitNormal);
			lua_setfield(L, -2, "hitNormal");

			return 1;
		}
	}

    return 0;
}

// TODO: Fix actual collider argument after adding new(__FILE__, __LINE__) components
int PhysicsEngineLua::lua_testContact(lua_State* L)
{
	PhysicsEngine* physicsEngine = (PhysicsEngine*)lua_touserdata(L, lua_upvalueindex(1));

	if (!lua_isnil(L, 1), lua_isvector(L, 2))
	{
		Collider col = Collider::createSphere(1.0f);
		glm::vec3 rot = lua_isvector(L, 3) ? lua_tovector(L, 3) : glm::vec3(0.0f);

		std::vector<Entity> entities = physicsEngine->testContact(col, lua_tovector(L, 2), rot);

		lua_newtable(L);
		for (int i = 0, end = (int)entities.size(); i < end; i++)
		{
			lua_pushinteger(L, entities[i]);
			lua_rawseti(L, -2, i + 1);
		}
		return 1;
	}

    return 0;
}

void PhysicsEngineLua::lua_openphysics(lua_State* L, PhysicsEngine* physicsEngine)
{
	lua_newtable(L);

	luaL_Reg methods[] = {
		{ "renderDebugShapes", lua_renderDebugShapes },
		{ "raycast", lua_raycast },
		{ "testContact", lua_testContact },
		{ NULL , NULL }
	};

	lua_pushlightuserdata(L, physicsEngine);
	luaL_setfuncs(L, methods, 1);
	lua_setglobal(L, "physics");

	lua_newtable(L);
	for (size_t i = 0; i < colliderTypes.size(); i++)
	{
		lua_pushnumber(L, (int)i);
		lua_setfield(L, -2, colliderTypes[i].c_str());
	}
	lua_setglobal(L, "ColliderType");
}
