#pragma once 
 #include "op_overload.hpp"

#include "../../network/NetworkHandler.h"
#include "../dev/LuaHelper.hpp"
#include "../LuaPushes.hpp"

class NetworkHandlerLua
{
   private:
	static int lua_sendPolygons(lua_State* L);
	static int lua_isServer(lua_State* L);
	static int lua_sendTCPData(lua_State* L);
	static int lua_sendUDPData(lua_State* L);
	static int lua_getNetworkData(lua_State* L);
	static int lua_sendPlayer(lua_State* L);

   public:
	static void lua_openNetworkScene(
	    lua_State* L, NetworkHandler* networkHandler
	);
};
