#include "Scene.hpp"
#include "SceneHandler.hpp"
#include "../systems/UpdateMatricesSystem.hpp"
#include "../network/NetworkHandler.h"
#include "Time.hpp"

void Scene::switchScene(std::string& path)
{
	this->sceneHandler->setScene(path);
}

NetworkHandler* Scene::getNetworkHandler()
{
	return sceneHandler->getNetworkHandler();
}

Scene::Scene()
	: sceneHandler(nullptr), mainCamera(-1)
{
	this->reg.clear();
	this->createSystem<UpdateMatricesSystem>();
}

Scene::~Scene()
{
	for (size_t i = 0; i < this->systems.size(); ++i)
	{
		delete this->systems[i];
	}
	this->systems.clear();
}

Camera* Scene::getMainCamera()
{
	Camera* cam = nullptr;
	if (this->entityValid(this->mainCamera)) { cam = &this->getComponent<Camera>(this->mainCamera); }
	return cam;
}

int Scene::getMainCameraID()
{
	return this->mainCamera;
}

void Scene::setMainCamera(int entity)
{
	if (this->hasComponents<Camera>(entity)) { this->mainCamera = entity; }
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

bool Scene::entityValid(int entity) const
{
	return this->reg.valid((entt::entity)entity);
}

int Scene::createEntity()
{
	int entity = (int)this->reg.create();
	this->setComponent<Transform>(entity);
	return entity;
}

bool Scene::removeEntity(int entity)
{
	bool valid = this->entityValid(entity);
	if (valid) { this->reg.destroy((entt::entity)entity); }
	return valid;
}

void Scene::setSceneHandler(SceneHandler& sceneHandler)
{
	this->sceneHandler = &sceneHandler;
}
