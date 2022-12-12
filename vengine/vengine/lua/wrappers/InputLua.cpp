#include "pch.h"
#include "InputLua.h"
#include "../LuaPushes.hpp"

int InputLua::lua_isKeyDown(lua_State* L)
{
	int key = (int)lua_tointeger(L, 1);
	lua_pushboolean(L, Input::isKeyDown((Keys)key));
	return 1;
}

int InputLua::lua_isKeyUp(lua_State* L)
{
	int key = (int)lua_tointeger(L, 1);
	lua_pushboolean(L, !Input::isKeyDown((Keys)key));
	return 1;
}

int InputLua::lua_isKeyPressed(lua_State* L)
{
	int key = (int)lua_tointeger(L, 1);
	lua_pushboolean(L, Input::isKeyPressed((Keys)key));
	return 1;
}

int InputLua::lua_isKeyReleased(lua_State* L)
{
	int key = (int)lua_tointeger(L, 1);
	lua_pushboolean(L, Input::isKeyReleased((Keys)key));
	return 1;
}

int InputLua::lua_isMouseButtonDown(lua_State* L)
{
	int button = (int)lua_tointeger(L, 1);
	lua_pushboolean(L, Input::isMouseButtonDown((Mouse)button));
	return 1;
}

int InputLua::lua_isMouseButtonUp(lua_State* L)
{
	int button = (int)lua_tointeger(L, 1);
	lua_pushboolean(L, !Input::isMouseButtonDown((Mouse)button));
	return 1;
}

int InputLua::lua_isMouseButtonPressed(lua_State* L)
{
	int button = (int)lua_tointeger(L, 1);
	lua_pushboolean(L, Input::isMouseButtonPressed((Mouse)button));
	return 1;
}

int InputLua::lua_isMouseButtonReleased(lua_State* L)
{
	int button = (int)lua_tointeger(L, 1);
	lua_pushboolean(L, Input::isMouseButtonReleased((Mouse)button));
	return 1;
}

int InputLua::lua_getMousePosition(lua_State* L)
{
	glm::vec2 pos = Input::getMouseUITranslated();
	lua_pushvector(L, { pos.x, pos.y, 0.0f });
	return 1;
}

int InputLua::lua_getMouseDelta(lua_State* L)
{
	glm::vec2 pos{ (float)Input::getMouseDeltaX(), (float)Input::getMouseDeltaY() };
	lua_pushvector(L, { pos.x, pos.y, 0.0f });
	return 1;
}

int InputLua::lua_getScrollWheelDelta(lua_State* L)
{
	lua_pushinteger(L, Input::getScrollWheelDelta());
	return 1;
}

int InputLua::lua_setCursorPosition(lua_State* L)
{
	if (lua_isvector(L, 1))
	{
		glm::vec3 vec = lua_tovector(L, 1);
		Input::setCursorPosition(vec.x, vec.y);
	}
	return 0;
}

int InputLua::lua_setHideCursor(lua_State* L)
{
	if (lua_isboolean(L, 1))
	{
		Input::setHideCursor(lua_toboolean(L, 1));
	}
	return 0;
}

void InputLua::lua_openinput(lua_State* L)
{
	lua_newtable(L);

	luaL_Reg methods[] = {
		{ "isKeyDown", lua_isKeyDown },
		{ "isKeyUp", lua_isKeyUp },
		{ "isKeyPressed", lua_isKeyPressed },
		{ "isKeyReleased", lua_isKeyReleased },
		{ "isMouseButtonDown", lua_isMouseButtonDown },
		{ "isMouseButtonUp", lua_isMouseButtonUp },
		{ "isMouseButtonPressed", lua_isMouseButtonPressed },
		{ "isMouseButtonReleased", lua_isMouseButtonReleased },
		{ "getMousePosition", lua_getMousePosition },
		{ "getMouseDelta", lua_getMouseDelta },
		{ "getScrollWheelDelta", lua_getScrollWheelDelta },
		{ "setCursorPosition", lua_setCursorPosition },
		{ "setHideCursor", lua_setHideCursor },
		{ NULL , NULL }
	};

	luaL_setfuncs(L, methods, 0);
	lua_setglobal(L, "input");

	lua_newtable(L);
	for (auto& element : Input::keysMap)
	{
		lua_pushnumber(L, element.second);
		lua_setfield(L, -2, element.first.c_str());
	}
	lua_setglobal(L, "Keys");

	lua_newtable(L);
	for (auto& element : Input::mouseMap)
	{
		lua_pushnumber(L, element.second);
		lua_setfield(L, -2, element.first.c_str());
	}
	lua_setglobal(L, "Mouse");
}
