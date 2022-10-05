#pragma once

#include "audio/AudioHandler.h"
#include "network/NetworkHandler.h"
#include "application/SceneHandler.hpp"
#include "ResourceManagement/ResourceManager.hpp"
#include "application/MatrixHandler.hpp"

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
	AudioHandler audioHandler;
    MatrixHandler matrixHandler;
};


