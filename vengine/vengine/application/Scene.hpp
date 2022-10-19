#pragma once

#include "../systems/System.hpp"
#include "../components/Transform.hpp"
#include "../components/Camera.hpp"
#include "../components/AudioSource.hpp"
#include "../components/AudioListener.hpp"
#include "../resource_management/Configurator.hpp"
#include "../resource_management/ResourceManager.hpp"
#include "../lua/ScriptHandler.h"

#include <entt.hpp>
#include <vector>

typedef bool Inactive;
typedef int Entity;

class SceneHandler;
class NetworkHandler;
class ScriptHandler;
class UIRenderer;
class DebugRenderer;

struct LuaSystem
{
	std::string path;
	int luaRef;
};

class Scene
{
private:
	SceneHandler* sceneHandler;

	entt::registry reg;
	std::vector<System*> systems;
	std::vector<LuaSystem> luaSystems;
	Entity mainCamera;

protected:
	void switchScene(Scene* scene, std::string path = "");
	NetworkHandler* getNetworkHandler();
	ScriptHandler* getScriptHandler();
	ResourceManager* getResourceManager();
	UIRenderer* getUIRenderer();
	DebugRenderer* getDebugRenderer();

    template <typename T> T getConfigValue(std::string_view name)
    {
        return vengine_helper::config::DEF<T>(name);
    }

public:
	Scene();
	virtual ~Scene();

	Camera* getMainCamera();
	Entity getMainCameraID();
	void setMainCamera(Entity entity);

	void createSystem(std::string& path);
	void setScriptComponent(Entity entity, std::string path);

	template <typename T, typename ...Args>
	void createSystem(Args... args);

	void updateSystems();

	int getEntityCount() const;
	bool entityValid(Entity entity) const;

	Entity createEntity();
	bool removeEntity(Entity entity);

	template <typename ...Args>
	bool hasComponents(Entity entity);

	template <typename T>
	T& getComponent(Entity entity);

	template <typename T>
	void setComponent(Entity entity, const T&);

	template <typename T, typename ...Args>
	void setComponent(Entity entity, Args... args);

	template <typename T>
	void removeComponent(Entity entity);

	void setActive(Entity entity);
	void setInactive(Entity entity);
	bool isActive(Entity entity);

	virtual void init();
	virtual void update();

	inline entt::registry& getSceneReg() { return this->reg; }
	inline std::vector<LuaSystem>& getLuaSystems() { return this->luaSystems; }

	void setSceneHandler(SceneHandler& sceneHandler);
};

template<typename T, typename ...Args>
inline void Scene::createSystem(Args ...args)
{
	this->systems.emplace_back(new T(args...));
}

template <typename ...Args>
bool Scene::hasComponents(Entity entity)
{
	return this->reg.all_of<Args...>(
		(entt::entity)entity);
}

template <typename T>
T& Scene::getComponent(Entity entity)
{
	return this->reg.get<T>(
		(entt::entity)entity);
}

template <typename T>
void Scene::setComponent(Entity entity,
	const T& component)
{
	this->reg.emplace_or_replace<T>(
		(entt::entity)entity, component);
}

template <typename T, typename ...Args>
void Scene::setComponent(Entity entity,
	Args... args)
{
	this->reg.emplace_or_replace<T>(
		(entt::entity)entity, args...);
}

template <typename T>
void Scene::removeComponent(Entity entity)
{
	// Don't remove transform
	if (typeid(T) == typeid(Transform))
	{ return; }

	this->reg.remove<T>((entt::entity)entity);
}