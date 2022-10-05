#pragma once

#include <string>
#include <iostream>
#include <lua.hpp>

#define LUA_ERR_CHECK(L, X) if(X != LUA_OK) { LuaH::dumpError(L); } 

namespace LuaH
{
	static void dumpStack(lua_State* L, bool popStack = false)
	{
		std::cout << "--------- STACK BEGIN ---------\n\n";

		int size = lua_gettop(L);
		for (int i = size; i > 0; i--)
		{
			int type = lua_type(L, i);
			std::string typeStr = lua_typename(L, type);
			std::string val;
			switch (type)
			{
			case LUA_TNIL:
				val = "nil";
				break;
			case LUA_TNUMBER:
				val = std::to_string(lua_tonumber(L, i));
				break;
			case LUA_TBOOLEAN:
				val = lua_toboolean(L, i) ? "true" : "false";
				break;
			case LUA_TSTRING:
				val = "\"";
				val += lua_tostring(L, i);

				if (val.size() > 11)
				{
					val.resize(9);
					val += "..";
				}
				val += "\"";
				break;
			case LUA_TFUNCTION:
				typeStr = "function";
				break;
			case LUA_TTABLE:
				typeStr = "table";
				break;
			default:
				typeStr = "";
				break;
			}

			std::cout << i << " | ";
			std::cout << typeStr << std::string(22 - typeStr.size() - val.size(), ' ') << val;
			std::cout << " | " << i - (size + 1) << std::endl;
		}
		if (popStack)
			lua_pop(L, size);

		std::cout << "\n---------- STACK END ----------\n";
	}

	static void dumpError(lua_State* L)
	{
		if (lua_gettop(L) && lua_isstring(L, -1))
		{
			std::cout << "Lua Error: " << lua_tostring(L, -1) << std::endl;
			lua_pop(L, 1);
		}
	}

	static void printTable(lua_State* L, int index)
	{
		lua_pushvalue(L, index);
		lua_pushnil(L);
		while (lua_next(L, -2))
		{
			lua_pushvalue(L, -2);

			const char* key = lua_tostring(L, -1);
			const char* value = lua_tostring(L, -2);
			printf("%s => %s\n", key, value);

			lua_pop(L, 2);
		}
		lua_pop(L, 1);
	}
}

