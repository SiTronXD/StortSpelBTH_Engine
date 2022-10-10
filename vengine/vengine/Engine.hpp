#pragma once

#include "application/SceneHandler.hpp"
#include "network/NetworkHandler.h"
#include "lua/ScriptHandler.h"
#include "audio/AudioHandler.h"
#include "resource_management/ResourceManager.hpp"
#include "graphics/UIRenderer.hpp"

class Engine
{
private:

public:
	Engine();
	virtual ~Engine();

	void run(std::string appName, std::string startScenePath, Scene* startScene = nullptr);

	SceneHandler    sceneHandler;
    ResourceManager resourceManager;
	NetworkHandler networkHandler;
	ScriptHandler scriptHandler;
	AudioHandler audioHandler;
	UIRenderer uiRenderer;
};


