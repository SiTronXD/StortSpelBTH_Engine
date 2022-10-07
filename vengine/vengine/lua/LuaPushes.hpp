#pragma once

#include "dev/LuaHelper.hpp"
#include "glm/glm.hpp"
#include "../components/Transform.hpp"
#include "../components/MeshComponent.hpp"

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

namespace TransformLua
{
	static int lua_right(lua_State* L)
	{
		Transform transform = lua_totransform(L, 1);
		transform.updateMatrix();
		lua_pushvector(L, transform.right());
		return 1;
	}

	static int lua_up(lua_State* L)
	{
		Transform transform = lua_totransform(L, 1);
		transform.updateMatrix();
		lua_pushvector(L, transform.up());
		return 1;
	}

	static int lua_forward(lua_State* L)
	{
		Transform transform = lua_totransform(L, 1);
		transform.updateMatrix();
		lua_pushvector(L, transform.forward());
		return 1;
	}
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

	luaL_Reg methods[] = {
		{ "right", TransformLua::lua_right },
		{ "up", TransformLua::lua_up },
		{ "forward", TransformLua::lua_forward },
		{ NULL , NULL }
	};

	luaL_setfuncs(L, methods, 0);
}

static MeshComponent lua_tomesh(lua_State* L, int index)
{
	MeshComponent mesh;
	// Sanity check
	if (!lua_istable(L, index)) {
		std::cout << "Error: not table" << std::endl;
		return mesh;
	}

	lua_getfield(L, index, "meshID");
	mesh.meshID = lua_tointeger(L, -1);
	lua_pop(L, 1);

	return mesh;
}

static void lua_pushmesh(lua_State* L, const MeshComponent& mesh)
{
	lua_newtable(L);

	lua_pushinteger(L, mesh.meshID);
	lua_setfield(L, -2, "meshID");
}