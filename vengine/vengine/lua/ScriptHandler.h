#pragma once

#include <string>

struct lua_State;
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
	bool loadScript(std::string& path);

	void update();
	void cleanup();
};

