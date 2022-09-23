#pragma once

#include "dev/LuaHelper.hpp"
#include "glm/glm.hpp"

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