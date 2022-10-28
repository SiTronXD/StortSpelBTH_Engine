#pragma once

#include "../dev/LuaHelper.hpp"
#include "../../application/Input.hpp"

class InputLua
{
private:
    static int lua_isKeyDown(lua_State* L);
    static int lua_isKeyUp(lua_State* L);
    static int lua_isKeyPressed(lua_State* L);
    static int lua_isKeyReleased(lua_State* L);

    static int lua_isMouseButtonDown(lua_State* L);
    static int lua_isMouseButtonUp(lua_State* L);
    static int lua_isMouseButtonPressed(lua_State* L);
    static int lua_isMouseButtonReleased(lua_State* L);
    static int lua_getMousePosition(lua_State* L);
    static int lua_getMouseDelta(lua_State* L);
    static int lua_getScrollWheelDelta(lua_State* L);

    static int lua_setCursorPosition(lua_State* L);
    static int lua_setHideCursor(lua_State* L);
public:
    static void lua_openinput(lua_State* L);
};

