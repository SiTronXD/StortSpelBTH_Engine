#include "Scene.hpp"
#include "SceneHandler.hpp"
#include "../network/NetworkHandler.h"
#include "../lua/ScriptHandler.h"
#include "../graphics/UIRenderer.hpp"
#include "../ai/AIHandler.hpp"
#include "Time.hpp"

void Scene::switchScene(Scene* scene, std::string path)
{
	this->sceneHandler->setScene(scene, path);
}

NetworkHandler* Scene::getNetworkHandler()
{
	return this->sceneHandler->getNetworkHandler();
}

ScriptHandler* Scene::getScriptHandler()
{
	return sceneHandler->getScriptHandler();
}

ResourceManager* Scene::getResourceManager()
{
	return this->sceneHandler->getResourceManager();
}

PhysicsEngine* Scene::getPhysicsEngine()
{
	return this->sceneHandler->getPhysicsEngine();
}

UIRenderer* Scene::getUIRenderer()
{
	return this->sceneHandler->getUIRenderer();
}

DebugRenderer* Scene::getDebugRenderer()
{
	return this->sceneHandler->getDebugRenderer();
}

SceneHandler* Scene::getSceneHandler()
{
	return this->sceneHandler;
}

AIHandler* Scene::getAIHandler()
{
    return this->sceneHandler->getAIHandler();   
}

Scene::Scene()
	: sceneHandler(nullptr), mainCamera(-1)
{
	this->reg.clear();
}

Scene::~Scene()
{
	for (size_t i = 0; i < this->systems.size(); ++i)
	{
		delete this->systems[i];
	}
	this->reg.clear();
	this->systems.clear();
	this->luaSystems.clear();
}

Camera* Scene::getMainCamera()
{
	Camera* cam = nullptr;
	if (this->entityValid(this->mainCamera)) { cam = &this->getComponent<Camera>(this->mainCamera); }
	return cam;
}

Entity Scene::getMainCameraID()
{
	return this->mainCamera;
}

void Scene::setMainCamera(Entity entity)
{
	if (this->hasComponents<Camera>(entity)) { this->mainCamera = entity; }
}

void Scene::createSystem(std::string& path)
{
	this->luaSystems.push_back(LuaSystem { path, -1 });
}

void Scene::setScriptComponent(Entity entity, std::string path)
{
	this->getScriptHandler()->setScriptComponent(entity, path);
}

void Scene::updateSystems()
{
	for (auto it = this->systems.begin(); it != this->systems.end();)
	{
		if ((*it)->update(this->reg, Time::getDT()))
		{
			delete (*it);
			it = this->systems.erase(it);
		}
		else
		{
			it++;
		}
	}
}

int Scene::getEntityCount() const
{
	return (int)this->reg.alive();
}

bool Scene::entityValid(Entity entity) const
{
	return this->reg.valid((entt::entity)entity);
}

Entity Scene::createEntity()
{
	int entity = (int)this->reg.create();
	this->setComponent<Transform>(entity);
	return entity;
}

bool Scene::removeEntity(Entity entity)
{
	bool valid = this->entityValid(entity);
	if (valid) { this->reg.destroy((entt::entity)entity); }
	return valid;
}

void Scene::setInactive(Entity entity)
{
	this->setComponent<Inactive>(entity);
}

void Scene::setActive(Entity entity)
{
	this->removeComponent<Inactive>(entity);
}

bool Scene::isActive(Entity entity)
{
	return !this->hasComponents<Inactive>(entity);
}

void Scene::init()
{

}

void Scene::start()
{

}

void Scene::update()
{

}

void Scene::setSceneHandler(SceneHandler& sceneHandler)
{
	this->sceneHandler = &sceneHandler;
}
