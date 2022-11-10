#pragma once

#include "../../graphics/UIRenderer.hpp"
#include "../LuaPushes.hpp"

class UIRendererLua
{
private:
	inline static const std::vector<std::string> stringAlignment {
		"Left",
		"Center",
		"Right"
	};

	static int lua_setTexture(lua_State* L);
	static int lua_setBitmapFont(lua_State* L);
	static int lua_renderTexture(lua_State* L);
	static int lua_renderTexture3D(lua_State* L);
	static int lua_renderString(lua_State* L);
	static int lua_renderString3D(lua_State* L);
public:
	static void lua_openui(lua_State* L, UIRenderer* uiRenderer);
};

