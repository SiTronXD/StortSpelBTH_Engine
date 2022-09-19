#pragma once

#include "../Systems/System.hpp"
#include "../Components/Transform.hpp"
#include "../Components/Camera.hpp"

#include <entt.hpp>
#include <vector>

class SceneHandler;

class Scene
{
private:
	SceneHandler* sceneHandler;

	entt::registry reg;
	std::vector<System*> systems;
	int mainCamera;

protected:
	void switchScene(Scene* nextScene);

public:
	Scene();
	virtual ~Scene();

	Camera* getMainCamera();
	int getMainCameraID();
	void setMainCamera(int entity);

	template <typename T, typename ...Args>
	void createSystem(Args... args);

	void updateSystems();

	int getEntityCount() const;
	bool entityValid(int entity) const;

	int createEntity();
	bool removeEntity(int entity);

	template <typename ...Args>
	bool hasComponents(int entity);

	template <typename T>
	T& getComponent(int entity);

	template <typename T>
	void setComponent(int entity, const T&);

	template <typename T, typename ...Args>
	void setComponent(int entity, Args... args);

	template <typename T>
	void removeComponent(int entity);

	virtual void init() = 0;
	virtual void update() = 0;

	inline entt::registry& getSceneReg() { return this->reg; }

	void setSceneHandler(SceneHandler& sceneHandler);
};

template<typename T, typename ...Args>
inline void Scene::createSystem(Args ...args)
{
	this->systems.emplace_back(new T(args...));
}

template <typename ...Args>
bool Scene::hasComponents(int entity)
{
	return this->reg.all_of<Args...>(
		(entt::entity)entity);
}

template <typename T>
T& Scene::getComponent(int entity)
{
	return this->reg.get<T>(
		(entt::entity)entity);
}

template <typename T>
void Scene::setComponent(int entity,
	const T& component)
{
	this->reg.emplace_or_replace<T>(
		(entt::entity)entity, component);
}

template <typename T, typename ...Args>
void Scene::setComponent(int entity,
	Args... args)
{
	this->reg.emplace_or_replace<T>(
		(entt::entity)entity, args...);
}

template <typename T>
void Scene::removeComponent(int entity)
{
	// Don't remove transform
	if (typeid(T) == typeid(Transform))
	{ return; }

	this->reg.remove<T>((entt::entity)entity);
}