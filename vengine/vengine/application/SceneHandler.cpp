#include "SceneHandler.hpp"
#include "Time.hpp"
#include "../components/AnimationComponent.hpp"
#include "../graphics/VulkanRenderer.hpp"
#include "../lua/ScriptHandler.h"

void SceneHandler::initSubsystems()
{
	this->physicsEngine->init();

	// Init scene
  this->scene->init();
  if (this->luaScript.size() != 0)
  {
    this->scriptHandler->runScript(this->luaScript);
  }
  this->scene->start();

  // Init renderer for scene
  this->vulkanRenderer->initForScene(this->scene);
  
  // Reset delta time counter
  Time::reset();
}

SceneHandler::SceneHandler()
	: scene(nullptr), 
	nextScene(nullptr), 
	networkHandler(nullptr), 
	scriptHandler(nullptr),
	resourceManager(nullptr),
	vulkanRenderer(nullptr),
	uiRenderer(nullptr)
{ }

SceneHandler::~SceneHandler()
{
	delete this->scene;
}

void SceneHandler::update()
{
	this->scene->updateSystems();
	this->scriptHandler->updateSystems(this->scene->getLuaSystems());
	this->scene->update();

	// Update animation timer
	auto animView = this->scene->getSceneReg().view<AnimationComponent>(entt::exclude<Inactive>);
	animView.each([&]
			(AnimationComponent& animationComponent)
		{
			animationComponent.timer += Time::getDT() * 24.0f * animationComponent.timeScale;
			if (animationComponent.timer >= animationComponent.endTime)
			{
				animationComponent.timer -= animationComponent.endTime;
			}
		}
	);
	auto view = this->scene->getSceneReg().view<Transform>(entt::exclude<Inactive>);
	auto func = [](Transform& transform)
	{
		transform.updateMatrix();
	};
	view.each(func);
}

void SceneHandler::updateToNextScene()
{
	// Make sure a scene can be switched to
	if (this->nextScene != nullptr)
	{
		// Delete old scene
		delete this->scene;
		this->scene = nullptr;

		// Switch
		this->scene = this->nextScene;
		this->nextScene = nullptr;
		this->luaScript = this->nextLuaScript;

		// Init 
		this->initSubsystems();
	}
}

void SceneHandler::setScene(Scene* scene, std::string path)
{
	if (this->nextScene == nullptr)
	{
		this->nextScene = scene;
		this->nextScene->setSceneHandler(*this);
		this->nextLuaScript = path;
	}
}

void SceneHandler::reloadScene()
{
	// Clear registry
	this->scene->getSceneReg().clear();
	
	// Init
	this->initSubsystems();
}

void SceneHandler::setNetworkHandler(NetworkHandler* networkHandler)
{
	this->networkHandler = networkHandler;
}

void SceneHandler::setScriptHandler(ScriptHandler* scriptHandler)
{
	this->scriptHandler = scriptHandler;
}

ScriptHandler* SceneHandler::getScriptHandler()
{
	return this->scriptHandler;
}

void SceneHandler::setPhysicsEngine(PhysicsEngine* physicsEngine)
{
	this->physicsEngine = physicsEngine;
}

void SceneHandler::setResourceManager(ResourceManager* resourceManager)
{
	this->resourceManager = resourceManager;
}

void SceneHandler::setUIRenderer(UIRenderer* uiRenderer)
{
	this->uiRenderer = uiRenderer;
}

UIRenderer* SceneHandler::getUIRenderer()
{
	return this->uiRenderer;
}

void SceneHandler::setDebugRenderer(DebugRenderer* debugRenderer)
{
	this->debugRenderer = debugRenderer;
}

DebugRenderer* SceneHandler::getDebugRenderer()
{
	return this->debugRenderer;
}

void SceneHandler::setVulkanRenderer(VulkanRenderer* vulkanRenderer)
{
	this->vulkanRenderer = vulkanRenderer;
}

VulkanRenderer* SceneHandler::getVulkanRenderer()
{
	return this->vulkanRenderer;
}

Scene* SceneHandler::getScene() const
{
	return this->scene;
}
