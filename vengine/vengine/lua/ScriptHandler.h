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

public:
	ScriptHandler();
	virtual ~ScriptHandler();

	void setSceneHandler(SceneHandler* sceneHandler);

	void update();
};

