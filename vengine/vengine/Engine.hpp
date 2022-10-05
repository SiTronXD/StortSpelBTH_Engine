#pragma once

#include "application/SceneHandler.hpp"
#include "application/PhysicsEngine.h"

class Engine
{
private:

public:
	Engine();
	virtual ~Engine();

	void run(Scene* startScene);

	SceneHandler sceneHandler;
	PhysicsEngine physEngine;
};

