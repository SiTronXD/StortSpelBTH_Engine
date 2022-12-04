#pragma once 
 #include "op_overload.hpp"

#include "../../graphics/DebugRenderer.hpp"
#include "../LuaPushes.hpp"

class DebugRendererLua
{
private:
	static int lua_renderLine(lua_State* L);
	static int lua_renderSphere(lua_State* L);
	static int lua_renderBox(lua_State* L);
	static int lua_renderCapsule(lua_State* L);
	static int lua_renderSkeleton(lua_State* L);
public:
	static void lua_open_debug_renderer(lua_State* L, DebugRenderer* debugRenderer);
};

