#pragma once

#include "../dev/LuaHelper.hpp"
#include "../../resource_management/ResourceManager.hpp"

class ResourceManagerLua
{
private:
	static int lua_addMesh(lua_State* L);
	static int lua_addTexture(lua_State* L);
public:
	static void lua_openresources(lua_State* L, ResourceManager* resources);
};

