#pragma once

#include "Application/SceneHandler.hpp"

class Engine
{
private:

public:
	Engine();
	virtual ~Engine();

	void run(Scene* startScene);

	SceneHandler sceneHandler;
};

