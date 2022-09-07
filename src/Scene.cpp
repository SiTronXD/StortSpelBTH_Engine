#include "Scene.h"
#include "SceneHandler.h"

#include "Transform.h"

Scene::Scene(SceneHandler& sceneHandler)
	: sceneHandler(sceneHandler)
{
}

Scene::~Scene()
{
}

void Scene::updateSystems(float deltaTime)
{
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
