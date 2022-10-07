#include "SceneHandler.hpp"
#include "Time.hpp"
#include "../graphics/VulkanRenderer.hpp"
#include "../components/AnimationComponent.hpp"

SceneHandler::SceneHandler()
	: scene(nullptr),
	nextScene(nullptr),
	networkHandler(nullptr),
	resourceManager(nullptr)
{ }

SceneHandler::~SceneHandler()
{
	delete this->scene;
}

void SceneHandler::update()
{
	this->scene->updateSystems();
	this->scene->update();

	// Update animation timer
	auto animView = scene->getSceneReg().view<AnimationComponent>();
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
		this->scene->init();
		Time::init();
	}
}

void SceneHandler::setScene(Scene* scene)
{
	if (this->nextScene == nullptr)
	{
		this->nextScene = scene;
		this->nextScene->setSceneHandler(*this);
	}
}

void SceneHandler::setNetworkHandler(NetworkHandler* networkHandler)
{
	this->networkHandler = networkHandler;
}

void SceneHandler::setResourceManager(ResourceManager* resourceManager)
{
	this->resourceManager = resourceManager;
}

Scene* SceneHandler::getScene() const
{
	return this->scene;
}
