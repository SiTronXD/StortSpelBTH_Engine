#pragma once
#include "Scene.hpp"

class SceneHandler
{
private:
	Scene* scene;
	Scene* nextScene;
	
public:
	SceneHandler();
	virtual ~SceneHandler();

	void update();
	void updateToNextScene();

	void setScene(Scene* scene);

	Scene* getScene() const;
};
