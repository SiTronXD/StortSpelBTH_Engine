#include "DebugRendererLua.h"

int DebugRendererLua::lua_renderLine(lua_State* L)
{
	DebugRenderer* debugRenderer = (DebugRenderer*)lua_touserdata(L, lua_upvalueindex(1));

	debugRenderer->renderLine(lua_tovector(L, 1), lua_tovector(L, 2), lua_tovector(L, 3));

	return 0;
}

int DebugRendererLua::lua_renderSphere(lua_State* L)
{
	DebugRenderer* debugRenderer = (DebugRenderer*)lua_touserdata(L, lua_upvalueindex(1));

	if (lua_isnumber(L, 2))
	{
		debugRenderer->renderSphere(lua_tovector(L, 1), (float)lua_tonumber(L, 2), lua_tovector(L, 3));
	}

	return 0;
}

int DebugRendererLua::lua_renderBox(lua_State* L)
{
	DebugRenderer* debugRenderer = (DebugRenderer*)lua_touserdata(L, lua_upvalueindex(1));

	debugRenderer->renderBox(lua_tovector(L, 1), lua_tovector(L, 2), lua_tovector(L, 3), lua_tovector(L, 4));

	return 0;
}

int DebugRendererLua::lua_renderCapsule(lua_State* L)
{
	DebugRenderer* debugRenderer = (DebugRenderer*)lua_touserdata(L, lua_upvalueindex(1));

	if (lua_isnumber(L, 3) && lua_isnumber(L, 4))
	{
		debugRenderer->renderCapsule(lua_tovector(L, 1), lua_tovector(L, 2), (float)lua_tonumber(L, 3), (float)lua_tonumber(L, 4), lua_tovector(L, 5));
	}

	return 0;
}

int DebugRendererLua::lua_renderSkeleton(lua_State* L)
{
	DebugRenderer* debugRenderer = (DebugRenderer*)lua_touserdata(L, lua_upvalueindex(1));

	if (lua_isnumber(L, 1))
	{
		debugRenderer->renderSkeleton((int)lua_tonumber(L, 1), lua_tovector(L, 2));
	}

	return 0;
}

void DebugRendererLua::lua_open_debug_renderer(lua_State* L, DebugRenderer* debugRenderer)
{
	lua_newtable(L);

	luaL_Reg methods[] = {
		{ "renderLine", lua_renderLine },
		{ "renderSphere", lua_renderSphere },
		{ "renderBox", lua_renderBox },
		{ "renderCapsule", lua_renderCapsule },
		{ "renderSkeleton", lua_renderSkeleton },
		{ NULL , NULL }
	};

	lua_pushlightuserdata(L, debugRenderer);
	luaL_setfuncs(L, methods, 1);
	lua_setglobal(L, "debugRenderer");
}
