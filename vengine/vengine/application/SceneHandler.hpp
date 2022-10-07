#pragma once
#include "Scene.hpp"

class SceneHandler
{
private:
	Scene* scene;
	Scene* nextScene;
	std::string luaScript;
	std::string nextLuaScript;

	NetworkHandler* networkHandler;
	ScriptHandler* scriptHandler;
	ResourceManager* resourceManager;
	
public:
	SceneHandler();
	virtual ~SceneHandler();

	void update();
	void updateToNextScene();

	void setScene(Scene* scene, std::string path = "");
	void reloadScene();

	void setNetworkHandler(NetworkHandler* networkHandler);
	void setResourceManager(ResourceManager* resourceManager);
	inline NetworkHandler* getNetworkHandler()
	{ return this->networkHandler; }
	inline ResourceManager* getResourceManager() 
	{ return this->resourceManager; }

	void setScriptHandler(ScriptHandler* scriptHandler);
	ScriptHandler* getScriptHandler();

	void setResourceManager(ResourceManager* resourceManager);
	ResourceManager* getResourceManager();

	Scene* getScene() const;
};
