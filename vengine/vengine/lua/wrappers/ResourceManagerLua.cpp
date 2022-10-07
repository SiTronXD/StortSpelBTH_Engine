#include "ResourceManagerLua.h"

int ResourceManagerLua::lua_addMesh(lua_State* L)
{
	ResourceManager* resourceManager = (ResourceManager*)lua_touserdata(L, lua_upvalueindex(1));

	if(!lua_isstring(L, 1)) { return 0; }

	int meshID = resourceManager->addMesh(lua_tostring(L, 1));
	lua_pushnumber(L, meshID);

	return 1;
}

int ResourceManagerLua::lua_addTexture(lua_State* L)
{
	ResourceManager* resourceManager = (ResourceManager*)lua_touserdata(L, lua_upvalueindex(1));

	if(!lua_isstring(L, 1)) { return 0; }

	int textureID = resourceManager->addTexture(lua_tostring(L, 1));
	lua_pushnumber(L, textureID);

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
}
