#pragma once
#include "Scene.hpp"

class SceneHandler
{
private:
	Scene* scene;
	Scene* nextScene;
	NetworkHandler* networkHandler;
	
public:
	SceneHandler();
	virtual ~SceneHandler();

	void update();
	void updateToNextScene();

	void setScene(Scene* scene);
	void setNetworkHandler(NetworkHandler* networkHandler);
	NetworkHandler* getNetworkHandler();

	Scene* getScene() const;
};
