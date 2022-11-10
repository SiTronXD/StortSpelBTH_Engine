#pragma once

#include <string>
#include <vector>

#include "../components/Script.hpp"

typedef int Entity;
struct lua_State;
struct LuaSystem;
enum class CallbackType;
class SceneHandler;
class ResourceManager;
class NetworkHandler;
class PhysicsEngine;
class UIRenderer;
class DebugRenderer;

class ScriptHandler
{
protected:
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
	void runFunction(Entity e, Script& script, const std::string& func);
	void setScriptComponent(Entity entity, std::string& path);
	void runCollisionFunction(Script& script, Entity e1, Entity e2, bool isTrigger, CallbackType type);
	void updateSystems(std::vector<LuaSystem>& vec);

	void update();
	void cleanup();

	bool getScriptComponentValue(Script& script, int& ret, const std::string& name);
	bool getScriptComponentValue(Script& script, float& ret, const std::string& name);
	bool getScriptComponentValue(Script& script, std::string& ret, const std::string& name);
	bool getScriptComponentValue(Script& script, glm::vec3& ret, const std::string& name);

	void setScriptComponentValue(Script& script, const int& val, const std::string& name);
	void setScriptComponentValue(Script& script, const float& val, const std::string& name);
	void setScriptComponentValue(Script& script, const std::string& val, const std::string& name);
	void setScriptComponentValue(Script& script, const glm::vec3& val, const std::string& name);

	bool getGlobal(int& ret, const std::string& name);
	bool getGlobal(float& ret, const std::string& name);
	bool getGlobal(std::string& ret, const std::string& name);
	bool getGlobal(glm::vec3& ret, const std::string& name);

	void setGlobal(const int& val, const std::string& name);
	void setGlobal(const float& val, const std::string& name);
	void setGlobal(const std::string& val, const std::string& name);
	void setGlobal(const glm::vec3& val, const std::string& name);
};