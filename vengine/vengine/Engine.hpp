#pragma once

#include "audio/AudioHandler.h"
#include "network/NetworkHandler.h"
#include "application/SceneHandler.hpp"

class Engine
{
private:

public:
	Engine();
	virtual ~Engine();

	void run(Scene* startScene);

	SceneHandler sceneHandler;
	NetworkHandler networkHandler;
	AudioHandler audioHandler;
};

