#pragma once

#include "SceneHandler.hpp"
#include "ResourceManager.hpp"
class Engine
{
private:

public:
	Engine();
	virtual ~Engine();

	void run(Scene* startScene);

    ResourceManager resourceMan;
	SceneHandler    sceneHandler;
};

