#pragma once
#pragma once

#include <string>
#include <vector>
#include "NetworkScene.h"
#include "../../lua/wrappers/SceneLua.h"

class NetworkSceneLua : public SceneLua
{
   private:

	static int lua_getPlayer(lua_State* L);
	static int lua_getPlayerCount(lua_State* L);
	static int lua_addEvent(lua_State* L);
	static int lua_PathFinding(lua_State* L);

   public:
	static void lua_openscene(lua_State* L, SceneHandler* sceneHandler);
};