#pragma once

#include <string>
#include <vector>

#include "../../components/Script.hpp"

typedef int Entity;
struct lua_State;
struct LuaSystem;
class NetworkScene;

class ServerScriptHandler
{
   private:
	static inline const std::string SCRIPT_PATH = "vengine_assets/scripts/";

	lua_State* L;
	NetworkScene* networkScene;

	void lua_openmetatables(lua_State* L);

	void updateScripts();

   public:
	ServerScriptHandler();
	virtual ~ServerScriptHandler();

	void setScene(NetworkScene* scene);

	bool runScript(std::string& path);
	void setScriptComponent(Entity entity, std::string& path);
	void updateSystems(std::vector<LuaSystem>& vec);

	void update();
	void cleanup();

	bool getScriptComponentValue(Script& script, int& ret, std::string name);
	bool getScriptComponentValue(Script& script, float& ret, std::string name);
	bool getScriptComponentValue(
	    Script& script, std::string& ret, std::string name
	);
};