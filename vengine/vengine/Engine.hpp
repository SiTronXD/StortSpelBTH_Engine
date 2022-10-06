#pragma once

#include "application/SceneHandler.hpp"
#include "network/NetworkHandler.h"
#include "lua/ScriptHandler.h"
#include "audio/AudioHandler.h"
#include "ResourceManagement/ResourceManager.hpp"
#include "application/MatrixHandler.hpp"

class Engine
{
private:

public:
	Engine();
	virtual ~Engine();

	void run(std::string appName, std::string startScenePath, Scene* startScene = nullptr);

	SceneHandler    sceneHandler;
    ResourceManager resourceMan;
	NetworkHandler networkHandler;
	ScriptHandler scriptHandler;
	AudioHandler audioHandler;
    MatrixHandler matrixHandler;
};


