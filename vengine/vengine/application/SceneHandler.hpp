#pragma once
#include "Scene.hpp"

class SceneHandler
{
private:
	Scene* scene;
	Scene* nextScene;
	NetworkHandler* networkHandler;
	ResourceManager* resourceManager;

public:
	SceneHandler();
	virtual ~SceneHandler();

	void update();
	void updateToNextScene();

	void setScene(Scene* scene);
	void setNetworkHandler(NetworkHandler* networkHandler);
	void setResourceManager(ResourceManager* resourceManager);
	inline NetworkHandler* getNetworkHandler()
	{ return this->networkHandler; }
	inline ResourceManager* getResourceManager() 
	{ return this->resourceManager; }

	Scene* getScene() const;
};
