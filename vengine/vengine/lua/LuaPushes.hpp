#pragma once

#include "dev/LuaHelper.hpp"
#include "glm/glm.hpp"
#include "../components/Transform.hpp"

static glm::vec3 lua_tovector(lua_State* L, int index)
{
	glm::vec3 vec{ 0.0f };
	// Sanity check
	if (!lua_istable(L, index)) {
		std::cout << "Error: not table" << std::endl;
		return vec;
	}

	lua_getfield(L, index, "x");
	vec.x = (float)lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "y");
	vec.y = (float)lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "z");
	vec.z = (float)lua_tonumber(L, -1);
	lua_pop(L, 1);

	return vec;
}

static void lua_pushvector(lua_State* L, const glm::vec3& vec)
{
	lua_newtable(L);

	lua_pushnumber(L, vec.x);
	lua_setfield(L, -2, "x");

	lua_pushnumber(L, vec.y);
	lua_setfield(L, -2, "y");

	lua_pushnumber(L, vec.z);
	lua_setfield(L, -2, "z");

	lua_getglobal(L, "vector");
	lua_setmetatable(L, -2);
}

static Transform lua_totransform(lua_State* L, int index)
{
	Transform transform;
	// Sanity check
	if (!lua_istable(L, index)) {
		std::cout << "Error: not table" << std::endl;
		return transform;
	}

	lua_getfield(L, index, "position");
	transform.position = lua_tovector(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "rotation");
	transform.rotation = lua_tovector(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "scale");
	transform.scale = lua_tovector(L, -1);
	lua_pop(L, 1);

	return transform;
}

static void lua_pushtransform(lua_State* L, const Transform& transform)
{
	lua_newtable(L);

	lua_pushvector(L, transform.position);
	lua_setfield(L, -2, "position");

	lua_pushvector(L, transform.rotation);
	lua_setfield(L, -2, "rotation");

	lua_pushvector(L, transform.scale);
	lua_setfield(L, -2, "scale");
}