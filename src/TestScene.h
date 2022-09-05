#pragma once

#include "Scene.h"

class TestScene : public Scene
{
private:
	
public:
	TestScene(SceneHandler& sceneHandler);
	virtual ~TestScene();

	// Inherited via Scene
	virtual void init() override;
	virtual void update() override;
	virtual void renderUI() override;
};
