#pragma once

#include "SceneHandler.hpp"
#include "NetworkHandler.h"

class Engine
{
private:

public:
	Engine();
	virtual ~Engine();

	void run(Scene* startScene);

	SceneHandler sceneHandler;
	NetworkHandler networkHandler;
};

