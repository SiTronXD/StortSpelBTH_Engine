#include "Scene.h"
#include "SceneHandler.h"
#include "UpdateMatricesSystem.hpp"
#include "Time.h"

Scene::Scene(SceneHandler& sceneHandler)
	: sceneHandler(sceneHandler)
{
	this->createSystem<UpdateMatricesSystem>();
}

Scene::~Scene()
{
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
