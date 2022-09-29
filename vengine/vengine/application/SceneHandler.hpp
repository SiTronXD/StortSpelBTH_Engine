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

	void setScene(std::string& path);
	void reloadScene();

	void setNetworkHandler(NetworkHandler* networkHandler);
	NetworkHandler* getNetworkHandler();

	void setScriptHandler(ScriptHandler* scriptHandler);

	Scene* getScene() const;
};
