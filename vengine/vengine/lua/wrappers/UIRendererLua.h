#pragma once

#include "../../graphics/UIRenderer.hpp"
#include "../LuaPushes.hpp"

class UIRendererLua
{
private:
	static int lua_setTexture(lua_State* L);
	static int lua_renderTexture(lua_State* L);
public:
	static void lua_openui(lua_State* L, UIRenderer* uiRenderer);
};

