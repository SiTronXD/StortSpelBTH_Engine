#pragma once

#include "network/NetworkHandler.h"
#include "application/SceneHandler.hpp"
#include "ResourceManagement/ResourceManager.hpp"

class Engine
{
private:

public:
	Engine();
	virtual ~Engine();

	void run(Scene* startScene);

	SceneHandler    sceneHandler;
    ResourceManager resourceMan;
	NetworkHandler networkHandler;
};


