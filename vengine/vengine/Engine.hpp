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


#if defined(_WIN32)  
#if defined(_STATISTICS) || defined(_CONSOLE)
#include "windows/StatisticsCollector.hpp"
#endif
#endif

class Engine
{
private:

public:
	Engine();
	virtual ~Engine();

	void setCustomNetworkHandler(NetworkHandler* networkHandler);
	void run(std::string appName, std::string startScenePath, Scene* startScene = nullptr);

	SceneHandler sceneHandler;
    ResourceManager resourceManager;
	NetworkHandler* networkHandler;
	PhysicsEngine physicsEngine;
	ScriptHandler scriptHandler;
	AudioHandler audioHandler;
	UIRenderer uiRenderer;
	DebugRenderer debugRenderer;
    AIHandler aiHandler;



#if defined  (_STATISTICS) || defined(_CONSOLE)
	float avgFPS = 0.0f;
	uint32_t elapsedFrames = 0u;
#if defined(_WIN32)    
	StatisticsCollector statsCollector;
#endif
#endif
};