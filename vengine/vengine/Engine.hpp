#pragma once

#include "application/SceneHandler.hpp"
#include "network/NetworkHandler.h"
#include "physics/PhysicsEngine.h"
#include "lua/ScriptHandler.h"
#include "audio/AudioHandler.h"
#include "resource_management/ResourceManager.hpp"
#include "graphics/UIRenderer.hpp"
#include "graphics/DebugRenderer.hpp"
#include "ai/AIHandler.hpp"

#ifdef _CONSOLE
#include "dev/StatisticsCollector.hpp"
#endif

class Engine
{
private:

public:
	Engine();
	virtual ~Engine();

	void run(std::string appName, std::string startScenePath, Scene* startScene = nullptr);

	SceneHandler sceneHandler;
    ResourceManager resourceManager;
	NetworkHandler networkHandler;
	PhysicsEngine physicsEngine;
	ScriptHandler scriptHandler;
	AudioHandler audioHandler;
	UIRenderer uiRenderer;
	DebugRenderer debugRenderer;
    AIHandler aiHandler;

#ifdef _CONSOLE
	StatisticsCollector statsCollector;
#endif
};