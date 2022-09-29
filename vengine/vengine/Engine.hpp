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

	void run(std::string appName, std::string path);

	SceneHandler sceneHandler;
	NetworkHandler networkHandler;
	ScriptHandler scriptHandler;
};

