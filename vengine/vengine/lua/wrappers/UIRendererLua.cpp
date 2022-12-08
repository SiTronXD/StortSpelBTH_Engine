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

int UIRendererLua::lua_setBitmapFont(lua_State* L)
{
	UIRenderer* uiRenderer = (UIRenderer*)lua_touserdata(L, lua_upvalueindex(1));

	if (lua_istable(L, 1) && lua_isvector(L, 3))
	{
		std::vector<std::string> characters;
		lua_pushvalue(L, 1);
		lua_pushnil(L);
		while (lua_next(L, -2))
		{
			lua_pushvalue(L, -2);
			characters.push_back(lua_tostring(L, -2));
			lua_pop(L, 2);
		}
		lua_pop(L, 1);

		// Using ID
		if (lua_isnumber(L, 2))
		{
			uiRenderer->setBitmapFont(characters, (uint32_t)lua_tonumber(L, 2), lua_tovector(L, 3));
		}
		// Using string
		else if (lua_isstring(L, 2))
		{
			uiRenderer->setBitmapFont(characters, uiRenderer->getResourceManager()->addTexture(lua_tostring(L, 2)), lua_tovector(L, 3));
		}
	}

	return 0;
}

int UIRendererLua::lua_renderTexture(lua_State* L)
{
	UIRenderer* uiRenderer = (UIRenderer*)lua_touserdata(L, lua_upvalueindex(1));

	if (lua_isvector(L, 1) && lua_isvector(L, 2))
	{
		glm::vec3 col = lua_isvector(L, 3) ? lua_tovector(L, 3) : glm::vec3(1.0f);
		float alpha = lua_isnumber(L, 4) ? lua_tonumber(L, 4) : 1.0f;
		uiRenderer->renderTexture(glm::vec2(lua_tovector(L, 1)), lua_tovector(L, 2), glm::uvec4(0, 0, 1, 1), glm::vec4(col, alpha));
	}
	return 0;
}

int UIRendererLua::lua_renderTexture3D(lua_State* L)
{
	UIRenderer* uiRenderer = (UIRenderer*)lua_touserdata(L, lua_upvalueindex(1));

	if (lua_isvector(L, 1) && lua_isvector(L, 2))
	{
		glm::vec3 col = lua_isvector(L, 3) ? lua_tovector(L, 3) : glm::vec3(1.0f);
		float alpha = lua_isnumber(L, 4) ? lua_tonumber(L, 4) : 1.0f;
		uiRenderer->renderTexture(lua_tovector(L, 1), lua_tovector(L, 2), glm::uvec4(0, 0, 1, 1), glm::vec4(col, alpha));
	}
	return 0;
}

int UIRendererLua::lua_renderString(lua_State* L)
{
	UIRenderer* uiRenderer = (UIRenderer*)lua_touserdata(L, lua_upvalueindex(1));

	if (lua_isstring(L, 1) && lua_isvector(L, 2) && lua_isvector(L, 3))
	{
		float charMargin = lua_isnumber(L, 4) ? lua_tonumber(L, 4) : 0.0f;
		StringAlignment alignment = lua_isnumber(L, 5) ? (StringAlignment)lua_tointeger(L, 5) : StringAlignment::CENTER;
		glm::vec3 col = lua_isvector(L, 6) ? lua_tovector(L, 6) : glm::vec3(1.0f);
		float alpha = lua_isnumber(L, 7) ? lua_tonumber(L, 7) : 1.0f;

		uiRenderer->renderString(lua_tostring(L, 1), glm::vec2(lua_tovector(L, 2)), lua_tovector(L, 3), charMargin, alignment, glm::vec4(col, alpha));
	}
	return 0;
}

int UIRendererLua::lua_renderString3D(lua_State* L)
{
	UIRenderer* uiRenderer = (UIRenderer*)lua_touserdata(L, lua_upvalueindex(1));

	if (lua_isstring(L, 1) && lua_isvector(L, 2) && lua_isvector(L, 3))
	{
		float charMargin = lua_isnumber(L, 4) ? lua_tonumber(L, 4) : 0.0f;
		StringAlignment alignment = lua_isnumber(L, 5) ? (StringAlignment)lua_tointeger(L, 5) : StringAlignment::CENTER;
		glm::vec3 col = lua_isvector(L, 6) ? lua_tovector(L, 6) : glm::vec3(1.0f);
		float alpha = lua_isnumber(L, 7) ? lua_tonumber(L, 7) : 1.0f;

		uiRenderer->renderString(lua_tostring(L, 1), lua_tovector(L, 2), lua_tovector(L, 3), charMargin, alignment, glm::vec4(col, alpha));
	}
	return 0;
}

void UIRendererLua::lua_openui(lua_State* L, UIRenderer* uiRenderer)
{
	lua_newtable(L);

	luaL_Reg methods[] = {
		{ "setTexture", lua_setTexture },
		{ "setBitmapFont", lua_setBitmapFont },
		{ "renderTexture", lua_renderTexture },
		{ "renderTexture3D", lua_renderTexture3D },
		{ "renderString", lua_renderString },
		{ "renderString3D", lua_renderString3D },
		{ NULL , NULL }
	};

	lua_pushlightuserdata(L, uiRenderer);
	luaL_setfuncs(L, methods, 1);
	lua_setglobal(L, "uiRenderer");

	lua_newtable(L);
	for (size_t i = 0; i < stringAlignment.size(); i++)
	{
		lua_pushinteger(L, (int)i - 1);
		lua_setfield(L, -2, stringAlignment[i].c_str());
	}
	lua_setglobal(L, "StringAlignment");
}
