#include "pch.h"
#include "NetworkSceneLua.h"

int NetworkSceneLua::lua_getPlayer(lua_State* L)
{
	NetworkScene* scene = ((NetworkScene*)lua_touserdata(L, lua_upvalueindex(1)));
	lua_pushnumber(L, scene->getPlayer((int)lua_tointeger(L, 1)));
	return 1;
}

int NetworkSceneLua::lua_getPlayerCount(lua_State* L)
{
	NetworkScene* scene = ((NetworkScene*)lua_touserdata(L, lua_upvalueindex(1)));
	lua_pushnumber(L, scene->getPlayerSize());
	return 1;
}

int NetworkSceneLua::lua_addEvent(lua_State* L)
{
	NetworkScene* scene = ((NetworkScene*)lua_touserdata(L, lua_upvalueindex(1)));
	std::vector<int> ints;
	std::vector<float> floats;
	ints.push_back(lua_tointeger(L, 1));
	//check if that was all

	//if not check if its a table
	//if its a table get that and return

	//else its a number
	int i = 2;
	bool done = false;
	while (lua_gettop(L) > 0 && !done)
	{
		if (!lua_istable(L, i))
		{
			ints.push_back(lua_tointeger(L, i));
		}
		else
		{
			//go trough table
			lua_pushnil(L);
			while (lua_next(L, -2))
			{
				lua_pushvalue(L, -2);
				const char* key = lua_tostring(L, -1);

				floats.push_back((int)lua_tonumber(L, -2));

				lua_pop(L, 2);
			}
			lua_pop(L, 1);
			done = true;
		}
		i++;
	}
	scene->addEvent(ints, floats);
	lua_pop(L, lua_gettop(L));
	return 0;
}

int NetworkSceneLua::lua_PathFinding(lua_State* L) {
	NetworkScene* scene = ((NetworkScene*)lua_touserdata(L, lua_upvalueindex(1)));
	glm::vec3 from = lua_tovector(L, 1);
	glm::vec3 to = lua_tovector(L, 2);

	lua_pushvector(L, scene->getPathFinder().getDirTo(from, to));

	return 1;
}

void NetworkSceneLua::lua_openscene(lua_State* L, SceneHandler* sceneHandler)
{
	lua_newtable(L);

	luaL_Reg methods[] = {
	    {"getPlayer", lua_getPlayer},
	    {"getPlayerCount", lua_getPlayerCount},
	    {"addEvent", lua_addEvent},
	    {"getDirectionTo", lua_PathFinding},
		{NULL, NULL}
	};

	lua_pushlightuserdata(L, sceneHandler->getScene());
	luaL_setfuncs(L, methods, 1);
	lua_setglobal(L, "netScene");

	SceneLua::lua_openscene(L, sceneHandler);
}
