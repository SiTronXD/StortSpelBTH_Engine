#pragma once

#include "../dev/LuaHelper.hpp"
#include "../../resource_management/ResourceManager.hpp"
#include <map>

class ResourceManagerLua
{
private:
	inline static const std::map<std::string, int> filterMap{
		{ "Nearest", VK_FILTER_NEAREST},
		{ "Linear", VK_FILTER_LINEAR},
		{ "CubicEXT", VK_FILTER_CUBIC_EXT},
		{ "CubicIMG", VK_FILTER_CUBIC_IMG},
	};

	static int lua_addMesh(lua_State* L);
	static int lua_addTexture(lua_State* L);
	static int lua_addAnimations(lua_State* L);
	static int lua_addAudio(lua_State* L);
public:
	static void lua_openresources(lua_State* L, ResourceManager* resources);
};

