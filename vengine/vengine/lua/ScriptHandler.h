#pragma once

#include <string>
#include <vector>

typedef int Entity;
struct lua_State;
struct LuaSystem;
class SceneHandler;
class ResourceManager;
class NetworkHandler;

class ScriptHandler
{
private:
	static inline const std::string SCRIPT_PATH = "vengine_assets/scripts/";

	lua_State* L;
	SceneHandler* sceneHandler;
	NetworkHandler* networkHandler;

	void lua_openmetatables(lua_State* L);

	void updateScripts();

public:
	ScriptHandler();
	virtual ~ScriptHandler();

	void setSceneHandler(SceneHandler* sceneHandler);
	void setResourceManager(ResourceManager* resourceManager);
	void setNetworkHandler(NetworkHandler* networkHandler);

	bool runScript(std::string& path);
	void setScriptComponent(Entity entity, std::string& path);
	void updateSystems(std::vector<LuaSystem>& vec);

	void update();
	void cleanup();
};

