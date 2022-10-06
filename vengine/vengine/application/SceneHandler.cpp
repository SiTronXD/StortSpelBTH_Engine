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
	this->scriptHandler->updateSystems(this->scene->getLuaSystems());
	this->scene->update();

	auto view = this->scene->getSceneReg().view<Transform>();
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

		this->scene->init();
		if (this->luaScript.size() != 0)
		{
			this->scriptHandler->runScript(this->luaScript);
		}

		Time::init();
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
	this->scene->getSceneReg().clear();

	this->scene->init();
	if (this->luaScript.size() != 0)
	{
		this->scriptHandler->runScript(this->luaScript);
	}
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

ScriptHandler* SceneHandler::getScriptHandler()
{
	return this->scriptHandler;
}

void SceneHandler::setResourceManager(ResourceManager* resourceManager)
{
	this->resourceManager = resourceManager;
}

ResourceManager * SceneHandler::getResourceManager()
{
	return this->resourceManager;
}

Scene* SceneHandler::getScene() const
{
	return this->scene;
}
