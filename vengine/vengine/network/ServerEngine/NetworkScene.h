#pragma once
#include "../../application/Scene.hpp"

class NetworkScene{
private:
	entt::registry reg;
	std::vector<System*> systems;
	std::vector<LuaSystem> luaSystems;
	ScriptHandler* scriptHandler;
protected:
	ScriptHandler* getScriptHandler();
public:
	NetworkScene();
	virtual ~NetworkScene();

	//we don't have a scene handler here so this is it instead
	void setScriptHandler(ScriptHandler* scriptHandler);

	void createSystem(std::string& path);
	void setScriptComponent(Entity entity, std::string path);

	template <typename T, typename ...Args>
	void createSystem(Args... args);

	void updateSystems(float dt);

	int getEntityCount() const;
	bool entityValid(Entity entity) const;

	Entity createEntity();
	bool removeEntity(Entity entity);
	void removeAllEntitys();

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
};

template<typename T, typename ...Args>
inline void NetworkScene::createSystem(Args ...args)
{
	this->systems.emplace_back(new T(args...));
}

template <typename ...Args>
bool NetworkScene::hasComponents(Entity entity)
{
	return this->reg.all_of<Args...>(
		(entt::entity)entity);
}

template <typename T>
T& NetworkScene::getComponent(Entity entity)
{
	return this->reg.get<T>(
		(entt::entity)entity);
}

template <typename T>
void NetworkScene::setComponent(Entity entity,
	const T& component)
{
	this->reg.emplace_or_replace<T>(
		(entt::entity)entity, component);
}

template <typename T, typename ...Args>
void NetworkScene::setComponent(Entity entity,
	Args... args)
{
	this->reg.emplace_or_replace<T>(
		(entt::entity)entity, args...);
}

template <typename T>
void NetworkScene::removeComponent(Entity entity)
{
	// Don't remove transform
	if (typeid(T) == typeid(Transform))
	{ return; }

	this->reg.remove<T>((entt::entity)entity);
}