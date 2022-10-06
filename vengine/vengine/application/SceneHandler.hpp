#pragma once
#include "Scene.hpp"

class ScriptHandler;

class SceneHandler
{
private:
	Scene* scene;
	Scene* nextScene;
	std::string luaScript;
	std::string nextLuaScript;

	NetworkHandler* networkHandler;
	ScriptHandler* scriptHandler;
	
public:
	SceneHandler();
	virtual ~SceneHandler();

	void update();
	void updateToNextScene();

	void setScene(Scene* scene, std::string path = "");
	void reloadScene();

	void setNetworkHandler(NetworkHandler* networkHandler);
	NetworkHandler* getNetworkHandler();

	void setScriptHandler(ScriptHandler* scriptHandler);
	ScriptHandler* getScriptHandler();

	Scene* getScene() const;
};
