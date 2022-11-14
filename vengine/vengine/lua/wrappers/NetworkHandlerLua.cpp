#include "pch.h"
#include "NetworkHandlerLua.h"

int NetworkHandlerLua::lua_sendPolygons(lua_State* L)
{
	//define things
	NetworkHandler* networkHandler =
	    ((NetworkHandler*)lua_touserdata(L, lua_upvalueindex(1)));
	std::vector<glm::vec2> points;

	lua_pushnil(L);

	while (lua_next(L, -2))
	{
		lua_pushvalue(L, -2);
		const char* key = lua_tostring(L, -1);
		glm::vec3 point = lua_tovector(L, -2);

		points.push_back(glm::vec2(point.x, point.z));

		lua_pop(L, 2);
	}

	lua_pop(L, 1);
	//if we are on the server side send
	if (networkHandler->hasServer())
	{
		networkHandler->sendAIPolygons(points);
	}
	return 0;
}

int NetworkHandlerLua::lua_isServer(lua_State* L)
{
	NetworkHandler* networkHandler =
	    ((NetworkHandler*)lua_touserdata(L, lua_upvalueindex(1)));

	lua_pushboolean(L, networkHandler->hasServer());

	return 1;
}

int NetworkHandlerLua::lua_sendTCPData(lua_State* L)
{
	NetworkHandler* networkHandler = ((NetworkHandler*)lua_touserdata(L, lua_upvalueindex(1)));

	int typeOfCall = (int)lua_tointeger(L, 1);
	int ints[3] = {0,0,0};
	int nrOfInts = 0;
	std::vector<float> floats;

	//check the first table
	lua_pushnil(L);
	while (lua_next(L, -2))
	{
		lua_pushvalue(L, -2);
		const char* key = lua_tostring(L, -1);
		
		ints[nrOfInts] = (int)lua_tointeger(L, -2);
		nrOfInts++;

		lua_pop(L, 2);
	}
	lua_pop(L, 1);

	//check second table
	lua_pushnil(L);
	while (lua_next(L, -2))
	{
		lua_pushvalue(L, -2);
		const char* key = lua_tostring(L, -1);
		floats.push_back((float)lua_tonumber(L, -2));

		lua_pop(L, 2);
	}
	lua_pop(L, 1);

	TCPPacketEvent ev;
	ev.ints[0] = ints[0];
	ev.ints[1] = ints[1];
	ev.ints[2] = ints[2];
	ev.floats = floats;
	ev.event = typeOfCall;
	ev.nrOfInts = nrOfInts;

	networkHandler->sendTCPDataToClient(ev);

	return 0;
}

int NetworkHandlerLua::lua_sendUDPData(lua_State* L)
{
	NetworkHandler* networkHandler = ((NetworkHandler*)lua_touserdata(L, lua_upvalueindex(1)));
	networkHandler->sendUDPDataToClient(lua_tovector(L, 1), lua_tovector(L, 2));
	return 0;
}

void l_pushTableFloat(lua_State* L, const char* key, float value) {
	lua_pushstring(L, key);
	lua_pushnumber(L, value);
	lua_settable(L, -3);
}
void l_pushTableInt(lua_State* L, const char* key, int value) {
	lua_pushstring(L, key);
	lua_pushinteger(L, value);
	lua_settable(L, -3);
}

int NetworkHandlerLua::lua_getNetworkData(lua_State* L)
{
	//nothing actually happens right now
	//NetworkHandler* networkHandler = ((NetworkHandler*)lua_touserdata(L, lua_upvalueindex(1)));
	//std::vector<int> ints;
	//std::vector<float> floats;
	//networkHandler->getLuaData(ints, floats);
	//std::string a = "ints";
	//for (int i = 0; i < ints.size(); i++)
	//{
	//	l_pushTableInt(L, "ints", ints[i]);
	//}
	//for (int i = 0; i < floats.size(); i++)
	//{
	//	l_pushTableFloat(L, "floats", floats[i]);
	//}

	return 0;
}

int NetworkHandlerLua::lua_sendPlayer(lua_State* L)
{
	NetworkHandler* networkHandler = ((NetworkHandler*)lua_touserdata(L, lua_upvalueindex(1)));
	networkHandler->setPlayerNetworkHandler(lua_tointeger(L,1));
	return 0;
}

void NetworkHandlerLua::lua_openNetworkScene(lua_State* L, NetworkHandler* networkHandler)
{
	lua_newtable(L);

	luaL_Reg methods[] = {
	    {"sendPolygons", lua_sendPolygons},
	    {"isServer", lua_isServer},
	    {"sendTCPData", lua_sendTCPData},
	    {"sendUDPData", lua_sendUDPData},
	    {"getNetworkData", lua_getNetworkData},
		{"sendPlayer" , lua_sendPlayer },
		{NULL, NULL}
	};

	lua_pushlightuserdata(L, networkHandler);
	luaL_setfuncs(L, methods, 1);
	lua_setglobal(L, "network");
}