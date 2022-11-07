#include "pch.h"
#include "UIRendererLua.h"
#include "../../resource_management/ResourceManager.hpp"

int UIRendererLua::lua_setTexture(lua_State* L)
{
	UIRenderer* uiRenderer = (UIRenderer*)lua_touserdata(L, lua_upvalueindex(1));

	// Using ID
	if (lua_isnumber(L, 1))
	{
		uiRenderer->setTexture((uint32_t)lua_tonumber(L, 1));
	}
	// Using string
	else if (lua_isstring(L, 1))
	{
		uiRenderer->setTexture(uiRenderer->getResourceManager()->addTexture(lua_tostring(L, 1)));
	}

	return 0;
}

int UIRendererLua::lua_renderTexture(lua_State* L)
{
	UIRenderer* uiRenderer = (UIRenderer*)lua_touserdata(L, lua_upvalueindex(1));

	if (lua_gettop(L) >= 4)
	{
		uiRenderer->renderTexture((float)lua_tonumber(L, 1), (float)lua_tonumber(L, 2), (float)lua_tonumber(L, 3), (float)lua_tonumber(L, 4));
	}
	return 0;
}

void UIRendererLua::lua_openui(lua_State* L, UIRenderer* uiRenderer)
{
	lua_newtable(L);

	luaL_Reg methods[] = {
		{ "setTexture", lua_setTexture },
		{ "renderTexture", lua_renderTexture },
		{ NULL , NULL }
	};

	lua_pushlightuserdata(L, uiRenderer);
	luaL_setfuncs(L, methods, 1);
	lua_setglobal(L, "uiRenderer");
}
