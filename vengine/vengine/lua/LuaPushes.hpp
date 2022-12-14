#pragma once

#include "dev/LuaHelper.hpp"
#include "glm/glm.hpp"
#include "../components/Transform.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/Script.hpp"
#include "../components/Camera.hpp"
#include "../components/Collider.h"
#include "../components/Rigidbody.h"
#include "../components/AnimationComponent.hpp"
#include "../components/AudioSource.h"

static bool lua_isvector(lua_State* L, int index)
{
	bool b = lua_istable(L, index);
	if (b)
	{
		lua_getmetatable(L, index);
		lua_getglobal(L, "vector");
		b = lua_rawequal(L, -1, -2);
		lua_pop(L, 2);
	}
	return b;
}

static glm::vec3 lua_tovector(lua_State* L, int index)
{
	glm::vec3 vec{ 0.0f };
	// Sanity check
	if (!lua_isvector(L, index)) {
		std::cout << "Error: not vector" << std::endl;
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
		std::cout << "Error: not transform-table" << std::endl;
		return transform;
	}

	lua_getfield(L, index, "position");
	transform.position = lua_tovector(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "rotation");
	transform.rotation = lua_tovector(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "scale");
	transform.scale = !lua_isnil(L, -1) ? lua_tovector(L, -1) : glm::vec3(1.0f);
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
		std::cout << "Error: not mesh-table" << std::endl;
		return mesh;
	}

	lua_getfield(L, index, "meshID");
	mesh.meshID = (int)lua_tointeger(L, -1);
	lua_pop(L, 1);

	return mesh;
}

static void lua_pushmesh(lua_State* L, const MeshComponent& mesh)
{
	lua_newtable(L);

	lua_pushinteger(L, mesh.meshID);
	lua_setfield(L, -2, "meshID");
}

static Camera lua_tocamera(lua_State* L, int index)
{
	Camera cam;
	// Sanity check
	if (!lua_istable(L, index)) {
		std::cout << "Error: not camera-table" << std::endl;
		return cam;
	}

	lua_getfield(L, index, "fov");
	cam.fov = !lua_isnil(L, -1) ? (float)lua_tonumber(L, -1) : 90.0f;
	lua_pop(L, 1);

	return cam;
}

static void lua_pushcamera(lua_State* L, const Camera& cam)
{
	lua_newtable(L);

	lua_pushnumber(L, cam.fov);
	lua_setfield(L, -2, "fov");
}

static Collider lua_tocollider(lua_State* L, int index)
{
	Collider col;
	// Sanity check
	if (!lua_istable(L, index)) {
		std::cout << "Error: not collider-table" << std::endl;
		return col;
	}

	lua_getfield(L, index, "type");
	ColType type = (ColType)lua_tointeger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "offset");
	glm::vec3 offset = !lua_isnil(L, -1) ? lua_tovector(L, -1) : glm::vec3(0.0f);
	lua_pop(L, 1);

	lua_getfield(L, index, "isTrigger");
	bool trigger = lua_toboolean(L, -1);
	lua_pop(L, 1);

	float radius = 0.0f;
	float height = 0.0f;
	glm::vec3 extents = glm::vec3(0.0f);
	switch (type)
	{
	case ColType::SPHERE:
		lua_getfield(L, index, "radius");
		radius = !lua_isnil(L, -1) ? (float)lua_tonumber(L, -1) : 1.0f;
		lua_pop(L, 1);

		col = Collider::createSphere(radius, offset, trigger);
		break;
	case ColType::BOX:
		lua_getfield(L, index, "extents");
		extents = !lua_isnil(L, -1) ? lua_tovector(L, -1) : glm::vec3(1.0f);
		lua_pop(L, 1);

		col = Collider::createBox(extents, offset, trigger);
		break;
	case ColType::CAPSULE:
		lua_getfield(L, index, "radius");
		radius = !lua_isnil(L, -1) ? (float)lua_tonumber(L, -1) : 1.0f;
		lua_pop(L, 1);

		lua_getfield(L, index, "height");
		height = !lua_isnil(L, -1) ? (float)lua_tonumber(L, -1) : 1.0f;
		lua_pop(L, 1);

		col = Collider::createCapsule(radius, height, offset, trigger);
		break;
	default:
		break;
	}

	return col;
}

static void lua_pushcollider(lua_State* L, const Collider& col)
{
	lua_newtable(L);

	lua_pushinteger(L, (int)col.type);
	lua_setfield(L, -2, "type");

	lua_pushboolean(L, col.isTrigger);
	lua_setfield(L, -2, "isTrigger");

	switch (col.type)
	{
	case ColType::SPHERE:
		lua_pushnumber(L, col.radius);
		lua_setfield(L, -2, "radius");
		break;
	case ColType::BOX:
		lua_pushvector(L, col.extents);
		lua_setfield(L, -2, "extents");
		break;
	case ColType::CAPSULE:
		lua_pushnumber(L, col.radius);
		lua_setfield(L, -2, "radius");

		lua_pushnumber(L, col.height);
		lua_setfield(L, -2, "height");
		break;
	default:
		break;
	}
}

static Rigidbody lua_torigidbody(lua_State* L, int index, bool assigned = false)
{
	Rigidbody rb;
	// Sanity check
	if (!lua_istable(L, index)) {
		std::cout << "Error: not rigidbody-table" << std::endl;
		return rb;
	}

	rb.assigned = assigned;

	lua_getfield(L, index, "mass");
	rb.mass = !lua_isnil(L, -1) ? (float)lua_tonumber(L, -1) : 1.0f;
	lua_pop(L, 1);

	lua_getfield(L, index, "gravityMult");
	rb.gravityMult = !lua_isnil(L, -1) ? (float)lua_tonumber(L, -1) : 1.0f;
	lua_pop(L, 1);

	lua_getfield(L, index, "friction");
	rb.friction = !lua_isnil(L, -1) ? (float)lua_tonumber(L, -1) : 1.0f;
	lua_pop(L, 1);

	lua_getfield(L, index, "posFactor");
	rb.posFactor = !lua_isnil(L, -1) ? lua_tovector(L, -1) : glm::vec3(1.0f);
	lua_pop(L, 1);

	lua_getfield(L, index, "rotFactor");
	rb.rotFactor = !lua_isnil(L, -1) ? lua_tovector(L, -1) : glm::vec3(1.0f);
	lua_pop(L, 1);

	lua_getfield(L, index, "acceleration");
	rb.acceleration = !lua_isnil(L, -1) ? lua_tovector(L, -1) : glm::vec3(0.0f);
	lua_pop(L, 1);

	lua_getfield(L, index, "velocity");
	rb.velocity = !lua_isnil(L, -1) ? lua_tovector(L, -1) : glm::vec3(0.0f);
	lua_pop(L, 1);

	return rb;
}

static void lua_pushrigidbody(lua_State* L, const Rigidbody& rb)
{
	lua_newtable(L);

	lua_pushnumber(L, rb.mass);
	lua_setfield(L, -2, "mass");

	lua_pushnumber(L, rb.gravityMult);
	lua_setfield(L, -2, "gravityMult");

	lua_pushnumber(L, rb.friction);
	lua_setfield(L, -2, "friction");

	lua_pushvector(L, rb.posFactor);
	lua_setfield(L, -2, "posFactor");

	lua_pushvector(L, rb.rotFactor);
	lua_setfield(L, -2, "rotFactor");

	lua_pushvector(L, rb.acceleration);
	lua_setfield(L, -2, "acceleration");

	lua_pushvector(L, rb.velocity);
	lua_setfield(L, -2, "velocity");
}

static AnimationSlot lua_toanimationslot(lua_State* L, int index)
{
	AnimationSlot slot{};

	// Sanity check
	if (!lua_istable(L, index)) {
		std::cout << "Error: not animationslot-table" << std::endl;
		return slot;
	}

	lua_getfield(L, index, "animationIndex");
	slot.animationIndex = (uint32_t)lua_tointeger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "timer");
	slot.timer = (float)lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "timeScale");
	slot.timeScale = !lua_isnil(L, -1) ? (float)lua_tonumber(L, -1) : 1.0f;
	lua_pop(L, 1);

	return slot;
}

static void lua_pushanimationslot(lua_State* L, const AnimationSlot& animSlot)
{
	lua_newtable(L);

	lua_pushinteger(L, animSlot.animationIndex);
	lua_setfield(L, -2, "animationIndex");

	lua_pushnumber(L, animSlot.timer);
	lua_setfield(L, -2, "timer");

	lua_pushnumber(L, animSlot.timeScale);
	lua_setfield(L, -2, "timeScale");
}

static AnimationComponent lua_toanimation(lua_State* L, int index, StorageBufferID boneID)
{
	AnimationComponent anim{};
	anim.boneTransformsID = boneID;

	// Sanity check
	if (!lua_istable(L, index)) {
		std::cout << "Error: not animation-table" << std::endl;
		return anim;
	}

	for(int i = 0; i < NUM_MAX_ANIMATION_SLOTS; i++)
	{
		lua_rawgeti(L, index, i);
		if(lua_isnil(L, -1)) // Not valid
		{
			lua_pop(L, 1);
			return anim;
		}

		lua_getfield(L, -1, "animationIndex");
		anim.aniSlots[i].animationIndex = (uint32_t)lua_tonumber(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, -1, "timer");
		anim.aniSlots[i].timer = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, -1, "timeScale");
		anim.aniSlots[i].timeScale = !lua_isnil(L, -1) ? (float)lua_tonumber(L, -1) : 1.0f;
		lua_pop(L, 2);
	}

	return anim;
}

static void lua_pushanimation(lua_State* L, const AnimationComponent& anim)
{
	lua_newtable(L);
	for (int i = 0; i < NUM_MAX_ANIMATION_SLOTS; i++)
	{
		lua_pushanimationslot(L, anim.aniSlots[i]);
		lua_rawseti(L, -2, i);
	}
}

static void lua_pushAnimationStatus(lua_State* L, const AnimationStatus& status)
{
	lua_newtable(L);

	lua_pushstring(L, status.animationName.c_str());
	lua_setfield(L, -2, "animationName");

	lua_pushnumber(L, status.timer);
	lua_setfield(L, -2, "timer");

	lua_pushnumber(L, status.timeScale);
	lua_setfield(L, -2, "timeScale");

	lua_pushnumber(L, status.endTime);
	lua_setfield(L, -2, "endTime");

	lua_pushboolean(L, status.finishedCycle);
	lua_setfield(L, -2, "finishedCycle");
}

static UIArea lua_touiarea(lua_State* L, int index)
{
	UIArea area{};
	glm::vec3 vec;

	// Sanity check
	if (!lua_istable(L, index)) {
		std::cout << "Error: not UIArea-table" << std::endl;
		return area;
	}

	lua_getfield(L, index, "position");
	vec = lua_tovector(L, -1);
	area.position = glm::ivec2((int)vec.x, (int)vec.y);
	lua_pop(L, 1);

	lua_getfield(L, index, "dimension");
	vec = lua_tovector(L, -1);
	area.dimension = glm::ivec2((int)vec.x, (int)vec.y);
	lua_pop(L, 1);

	lua_getfield(L, index, "clickButton");
	area.clickButton = !lua_isnil(L, -1) ? (Mouse)lua_tointeger(L, -1) : Mouse::LEFT;
	lua_pop(L, 1);

	return area;
}

namespace UIAreaLua
{
	static int lua_isHovering(lua_State* L)
	{
		UIArea area = lua_touiarea(L, 1);
		lua_pushboolean(L, area.isHovering());
		return 1;
	}

	static int lua_isClicking(lua_State* L)
	{
		UIArea area = lua_touiarea(L, 1);
		lua_pushboolean(L, area.isClicking());
		return 1;
	}

	static int lua_isHolding(lua_State* L)
	{
		UIArea area = lua_touiarea(L, 1);
		lua_pushboolean(L, area.isHolding());
		return 1;
	}
}

static void lua_pushuiarea(lua_State* L, const UIArea& area)
{
	lua_newtable(L);

	lua_pushvector(L, glm::vec3((float)area.position.x, (float)area.position.y, 0.0f));
	lua_setfield(L, -2, "position");

	lua_pushvector(L, glm::vec3((float)area.dimension.x, (float)area.dimension.y, 0.0f));
	lua_setfield(L, -2, "dimension");

	lua_pushinteger(L, (int)area.clickButton);
	lua_setfield(L, -2, "clickButton");
	
	luaL_Reg methods[] = {
		{ "isHovering", UIAreaLua::lua_isHovering },
		{ "isClicking", UIAreaLua::lua_isClicking },
		{ "isHolding", UIAreaLua::lua_isHolding },
		{ NULL , NULL }
	};

	luaL_setfuncs(L, methods, 0);
}