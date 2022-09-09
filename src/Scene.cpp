#include "Scene.h"
#include "SceneHandler.h"
#include "UpdateMatricesSystem.hpp"
#include "Time.h"

void Scene::switchScene(Scene* nextScene)
{
	this->sceneHandler.setScene(nextScene);
}

Scene::Scene(SceneHandler& sceneHandler)
	: sceneHandler(sceneHandler)
{
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
