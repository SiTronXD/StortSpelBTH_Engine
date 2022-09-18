#include "SceneHandler.hpp"
#include "Time.hpp"
#include "VulkanRenderer.hpp"

SceneHandler::SceneHandler()
	: scene(nullptr),
	nextScene(nullptr)
{ }

SceneHandler::~SceneHandler()
{
	delete this->scene;
	if (this->nextScene != nullptr) { delete this->nextScene; }
}

void SceneHandler::update()
{
	this->scene->updateSystems();
	this->scene->update();
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

Scene* SceneHandler::getScene() const
{
	return this->scene;
}
