#pragma once

#include <string>
#include <vector>

#include "../components/Script.hpp"

typedef int Entity;
struct lua_State;
struct LuaSystem;
class SceneHandler;
class ResourceManager;
class NetworkHandler;
class PhysicsEngine;
class UIRenderer;
class DebugRenderer;

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
	void setPhysicsEngine(PhysicsEngine* physicsEngine);
	void setUIRenderer(UIRenderer* uiRenderer);
	void setDebugRenderer(DebugRenderer* debugRenderer);

	bool runScript(std::string& path);
	void setScriptComponent(Entity entity, std::string& path);
	void updateSystems(std::vector<LuaSystem>& vec);

	void update();
	void cleanup();

	bool getScriptComponentValue(Script& script, int& ret, std::string name);
	bool getScriptComponentValue(Script& script, float& ret, std::string name);
	bool getScriptComponentValue(Script& script, std::string& ret, std::string name);

	bool getGlobal(int& ret, std::string& name);
	bool getGlobal(float& ret, std::string& name);
	bool getGlobal(std::string& ret, std::string& name);
};