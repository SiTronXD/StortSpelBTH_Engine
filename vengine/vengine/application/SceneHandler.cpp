#include "SceneHandler.hpp"
#include "Time.hpp"
#include "../graphics/VulkanRenderer.hpp"
#include "../lua/ScriptHandler.h"

SceneHandler::SceneHandler()
	: scene(nullptr), nextScene(nullptr), networkHandler(nullptr), scriptHandler(nullptr)
{ }

SceneHandler::~SceneHandler()
{
	delete this->scene;
}

void SceneHandler::update()
{
	this->scene->updateSystems();
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
		this->scriptHandler->runScript(this->luaScript);

		Time::init();
	}
}

void SceneHandler::setScene(std::string& path)
{
	if (this->nextScene == nullptr)
	{
		this->nextScene = new Scene();
		this->nextScene->setSceneHandler(*this);
		this->nextLuaScript = path;
	}
}

void SceneHandler::reloadScene()
{
	this->setScene(this->luaScript);
}

void SceneHandler::setNetworkHandler(NetworkHandler* networkHandler)
{
	this->networkHandler = networkHandler;
}

NetworkHandler* SceneHandler::getNetworkHandler()
{
	return this->networkHandler;
}

void SceneHandler::setScriptHandler(ScriptHandler* scriptHandler)
{
	this->scriptHandler = scriptHandler;
}

Scene* SceneHandler::getScene() const
{
	return this->scene;
}
