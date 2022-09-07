#pragma once

#include "Scene.h"

class TestScene : public Scene
{
private:
	int testEntity;
public:
	TestScene(SceneHandler& sceneHandler);
	virtual ~TestScene();

	// Inherited via Scene
	virtual void init() override;
	virtual void update() override;
};
