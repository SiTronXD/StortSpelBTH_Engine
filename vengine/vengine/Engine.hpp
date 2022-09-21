#pragma once

#include "application/SceneHandler.hpp"
#include "network/NetworkHandler.h"
#include "lua/ScriptHandler.h"

class Engine
{
private:

public:
	Engine();
	virtual ~Engine();

	void run(Scene* startScene);

	SceneHandler sceneHandler;
	NetworkHandler networkHandler;
	ScriptHandler scriptHandler;
};

