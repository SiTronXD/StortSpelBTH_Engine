#include "ResourceManagerLua.h"

#include "../../audio/AudioHandler.h" //TEMP

int ResourceManagerLua::lua_addMesh(lua_State* L)
{
	ResourceManager* resourceManager = (ResourceManager*)lua_touserdata(L, lua_upvalueindex(1));

	if(!lua_isstring(L, 1)) { return 0; }

	int meshID = resourceManager->addMesh(lua_tostring(L, 1));
	lua_pushinteger(L, meshID);

	return 1;
}

int ResourceManagerLua::lua_addTexture(lua_State* L)
{
	ResourceManager* resourceManager = (ResourceManager*)lua_touserdata(L, lua_upvalueindex(1));

	if(!lua_isstring(L, 1)) { return 0; }

	TextureSamplerSettings settings = { vk::Filter::eLinear };
	if (lua_istable(L, 2))
	{
		lua_getfield(L, 2, "filterMode");
		if (lua_isnumber(L, -1)) { settings.filterMode = (vk::Filter)lua_tonumber(L, -1); }
		lua_pop(L, 1);
	}

	int textureID = resourceManager->addTexture(lua_tostring(L, 1), settings);
	lua_pushinteger(L, textureID);

	return 1;
}

// TODO change to resource manager later
int ResourceManagerLua::lua_addAudio(lua_State* L)
{
	if (!lua_isstring(L, 1)) { return 0; }
	int audioID = AudioHandler::loadFile(lua_tostring(L, 1));

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
