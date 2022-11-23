#pragma once

#include "../components/AudioSource.h"
#include "../components/Camera.hpp"
#include "../components/Transform.hpp"
#include "../lua/ScriptHandler.h"
#include "../physics/PhysicsEngine.h"
#include "../resource_management/Configurator.hpp"
#include "../resource_management/ResourceManager.hpp"
#include "../physics/PhysicsEngine.h"
#include "../lua/ScriptHandler.h"
#include "Window.hpp"
#include "../systems/System.hpp"
#include "../audio/AudioHandler.h"

#include <entt.hpp>
#include <vector>

typedef bool Inactive;
typedef int Entity;

class SceneHandler;
class NetworkHandler;
class ScriptHandler;
class UIRenderer;
class DebugRenderer;
class AIHandler;

struct LuaSystem
{
	std::string path;
	int luaRef;
};

struct BloomSettings
{
	float bloomBufferLerpAlpha = 0.04f;
	uint32_t numBloomMipLevels = 4;
};

class Scene
{
private:
	SceneHandler* sceneHandler;
	entt::registry reg;
	Entity mainCamera;
	BloomSettings bloomSettings{};

protected:
	std::vector<System*> systems;
	std::vector<LuaSystem> luaSystems;

	void setBloomBufferLerpAlpha(const float& alpha);
	void setBloomNumMipLevels(const uint32_t& numBloomMipLevels);

protected:
	void switchScene(Scene* scene, std::string path = "");
	NetworkHandler* getNetworkHandler();
	ScriptHandler* getScriptHandler();
	ResourceManager* getResourceManager();
	PhysicsEngine* getPhysicsEngine();
	UIRenderer* getUIRenderer();
	DebugRenderer* getDebugRenderer();
	SceneHandler* getSceneHandler();
	AIHandler* getAIHandler();
	AudioHandler* getAudioHandler();

	template <typename T>
	T getConfigValue(std::string_view name)
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

	template <typename T, typename... Args>
	void createSystem(Args... args);

	void updateSystems();

	int getEntityCount() const;
	bool entityValid(Entity entity) const;

	Entity createEntity();
	bool removeEntity(Entity entity);

	template <typename... Args>
	bool hasComponents(Entity entity);

	template <typename T>
	T& getComponent(Entity entity);

	template <typename T>
	void setComponent(Entity entity, const T&);

	template <typename T, typename... Args>
	void setComponent(Entity entity, Args... args);

	template <typename T>
	void removeComponent(Entity entity);

	void setActive(Entity entity);
	void setInactive(Entity entity);
	bool isActive(Entity entity);

	void setAnimation(Entity entity, const std::string& animationName, bool resetTimer = true);

	// When created
	virtual void init();
	// When starting (after lua)
	virtual void start();
	virtual void update();

	// Collision callbacks
	virtual void onCollisionEnter(Entity e1, Entity e2);
	virtual void onCollisionStay(Entity e1, Entity e2);
	virtual void onCollisionExit(Entity e1, Entity e2);
	virtual void onTriggerEnter(Entity e1, Entity e2);
	virtual void onTriggerStay(Entity e1, Entity e2);
	virtual void onTriggerExit(Entity e1, Entity e2);

	inline entt::registry& getSceneReg() { return this->reg; }
	inline std::vector<LuaSystem>& getLuaSystems() { return this->luaSystems; }
	inline const BloomSettings& getBloomSettings() const { return this->bloomSettings; }

	void setSceneHandler(SceneHandler& sceneHandler);
};

template <typename T, typename... Args>
inline void Scene::createSystem(Args... args)
{
	this->systems.emplace_back(new T(args...));
}

template <typename... Args>
bool Scene::hasComponents(Entity entity)
{
	return this->reg.all_of<Args...>((entt::entity)entity);
}

template <typename T>
T& Scene::getComponent(Entity entity)
{
	return this->reg.get<T>((entt::entity)entity);
}

template <typename T>
void Scene::setComponent(Entity entity, const T& component)
{
	this->reg.emplace_or_replace<T>((entt::entity)entity, component);
}

template <typename T, typename... Args>
void Scene::setComponent(Entity entity, Args... args)
{
	this->reg.emplace_or_replace<T>((entt::entity)entity, args...);
}

template <typename T>
void Scene::removeComponent(Entity entity)
{
	// Don't remove transform
	if (typeid(T) == typeid(Transform))
	{
		return;
	}

	this->reg.remove<T>((entt::entity)entity);
}