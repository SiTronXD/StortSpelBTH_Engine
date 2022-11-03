#pragma once
#include "Scene.hpp"

class UIRenderer;
class DebugRenderer;
class Window;

class SceneHandler
{
private:
	Scene* scene;
	Scene* nextScene;
	std::string luaScript;
	std::string nextLuaScript;

	NetworkHandler* networkHandler;
	ScriptHandler* scriptHandler;
    AIHandler* aiHandler;
	ResourceManager* resourceManager;
	PhysicsEngine* physicsEngine;
	VulkanRenderer* vulkanRenderer;
	UIRenderer* uiRenderer;
	DebugRenderer* debugRenderer;
	Window* window;

	void initSubsystems();

public:
	SceneHandler();
	virtual ~SceneHandler();

	void update();
	void updateToNextScene();
	void prepareForRendering();

	void setScene(Scene* scene, std::string path = "");
	void reloadScene();

	void setNetworkHandler(NetworkHandler* networkHandler);
	inline NetworkHandler* getNetworkHandler() { return this->networkHandler; }
	inline ResourceManager* getResourceManager() { return this->resourceManager; }

	void setScriptHandler(ScriptHandler* scriptHandler);
	ScriptHandler* getScriptHandler();

    void setAIHandler(AIHandler* aiHandler);
    inline AIHandler* getAIHandler(){return this->aiHandler;};

	void setPhysicsEngine(PhysicsEngine* physicsEngine);
	inline PhysicsEngine* getPhysicsEngine() { return this->physicsEngine; }

	void setResourceManager(ResourceManager* resourceManager);

	void setUIRenderer(UIRenderer* uiRenderer);
	UIRenderer* getUIRenderer();

	void setDebugRenderer(DebugRenderer* debugRenderer);
	DebugRenderer* getDebugRenderer();

	void setVulkanRenderer(VulkanRenderer* vulkanRenderer);
	VulkanRenderer* getVulkanRenderer();

	void setWindow(Window* window);
	Window* getWindow();

	Scene* getScene() const;
};
