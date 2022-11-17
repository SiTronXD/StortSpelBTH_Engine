#include "pch.h"
#include "ResourceManagerLua.h"

#include "../../audio/AudioHandler.h" //TEMP

int ResourceManagerLua::lua_addMesh(lua_State* L)
{
	ResourceManager* resourceManager = (ResourceManager*)lua_touserdata(L, lua_upvalueindex(1));

	if(!lua_isstring(L, 1)) { return 0; }

	std::string texturesPath = "";
	if (lua_isstring(L, 2))
	{
		texturesPath = lua_tostring(L, 2);
	}

	int meshID = resourceManager->addMesh(lua_tostring(L, 1), std::move(texturesPath));
	lua_pushinteger(L, meshID);

	return 1;
}

int ResourceManagerLua::lua_addTexture(lua_State* L)
{
	ResourceManager* resourceManager = (ResourceManager*)lua_touserdata(L, lua_upvalueindex(1));

	if(!lua_isstring(L, 1)) { return 0; }

	TextureSettings settings{};
	if (lua_istable(L, 2))
	{
		// Sampler settings struct
		lua_getfield(L, 2, "samplerSettings");
		if (lua_istable(L, -1))
		{
			lua_getfield(L, -1, "filterMode");
			if (lua_isnumber(L, -1)) { settings.samplerSettings.filterMode = (vk::Filter)lua_tonumber(L, -1); }
			lua_pop(L, 1);

			lua_getfield(L, -1, "unnormalizedCoordinates");
			if (lua_isboolean(L, -1)) { settings.samplerSettings.unnormalizedCoordinates = lua_toboolean(L, -1); }
			lua_pop(L, 1);
		}
		lua_pop(L, 1);
		
		// Texture settings
		lua_getfield(L, 2, "keepCpuPixelInfo");
		if (lua_isboolean(L, -1)) { settings.keepCpuPixelInfo = lua_toboolean(L, -1); }
		lua_pop(L, 1);
	}

	int textureID = resourceManager->addTexture(lua_tostring(L, 1), settings);
	lua_pushinteger(L, textureID);

	return 1;
}

int ResourceManagerLua::lua_addAnimations(lua_State* L)
{
	ResourceManager* resourceManager = (ResourceManager*)lua_touserdata(L, lua_upvalueindex(1));

	if (!lua_istable(L, 1)) { return 0; }
	
	std::vector<std::string> paths;
	lua_pushvalue(L, 1);
	lua_pushnil(L);
	while (lua_next(L, -2))
	{
		lua_pushvalue(L, -2);
		paths.push_back(lua_tostring(L, -2));
		lua_pop(L, 2);
	}
	lua_pop(L, 1);

	std::string texturesPath = "";
	if (lua_isstring(L, 2))
	{
		texturesPath = lua_tostring(L, 2);
	}

	int meshID = resourceManager->addAnimations(paths, std::move(texturesPath));
	lua_pushinteger(L, meshID);

	return 1;
}

int ResourceManagerLua::lua_mapAnimations(lua_State* L)
{
	ResourceManager* resourceManager = (ResourceManager*)lua_touserdata(L, lua_upvalueindex(1));

	if (!lua_isnumber(L, 1) || !lua_istable(L, 2)) { return 0; }

	std::vector<std::string> names;
	lua_pushvalue(L, 2);
	lua_pushnil(L);
	while (lua_next(L, -2))
	{
		lua_pushvalue(L, -2);
		names.push_back(lua_tostring(L, -2));
		lua_pop(L, 2);
	}
	lua_pop(L, 1);

	lua_pushboolean(L, resourceManager->mapAnimations(lua_tointeger(L, 1), names));

	return 1;
}

// TODO change to resource manager later
int ResourceManagerLua::lua_addAudio(lua_State* L)
{
	ResourceManager* resourceManager = (ResourceManager*)lua_touserdata(L, lua_upvalueindex(1));

	if (!lua_isstring(L, 1)) { return 0; }
	int audioID = resourceManager->addSound(lua_tostring(L, 1));

	if (audioID < 0) { return 0; }
	lua_pushinteger(L, audioID);

	return 1;
}

void ResourceManagerLua::lua_openresources(lua_State* L, ResourceManager* resources)
{
	lua_newtable(L);

	luaL_Reg methods[] = {
		{ "addMesh", lua_addMesh },
		{ "addTexture", lua_addTexture },
		{ "addAnimations", lua_addAnimations },
		{ "mapAnimations", lua_mapAnimations },
		{ NULL , NULL }
	};

	lua_pushlightuserdata(L, resources);
	luaL_setfuncs(L, methods, 1);
	lua_setglobal(L, "resources");

	lua_newtable(L);
	for (auto& element : filterMap)
	{
		lua_pushnumber(L, element.second);
		lua_setfield(L, -2, element.first.c_str());
	}
	lua_setglobal(L, "Filters");
}
