#pragma once

#include <string>
#include <vector>

struct lua_State;
struct LuaSystem;
class SceneHandler;

class ScriptHandler
{
private:
	static inline const std::string SCRIPT_PATH = "vengine_assets/scripts/";

	lua_State* L;
	SceneHandler* sceneHandler;

	void lua_openmetatables(lua_State* L);

	void updateScripts();

public:
	ScriptHandler();
	virtual ~ScriptHandler();

	void setSceneHandler(SceneHandler* sceneHandler);

	bool runScript(std::string& path);
	void setScriptComponent(int entity, std::string& path);
	void updateSystems(std::vector<LuaSystem>& vec);

	void update();
	void cleanup();
};

