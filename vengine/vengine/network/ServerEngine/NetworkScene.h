#pragma once
#include "../../application/Scene.hpp"
#include "ServerScriptHandler.h"
#include <SFML/Network.hpp>
#include "../NetworkEnumAndDefines.h"

class NetworkScene{
private:
	entt::registry reg;
	std::vector<System*> systems;
	std::vector<LuaSystem> luaSystems;
	ServerScriptHandler* scriptHandler;

	std::vector<sf::Packet>* serverToClient;
	
						//id, type
	std::vector<std::pair<int,int>> enemies;
	std::vector<int> players;

   protected:
	ServerScriptHandler* getScriptHandler();

   public:
	NetworkScene();
	virtual ~NetworkScene();

	//custom function for server only
	int createPlayer();
	int getPlayer(const int &whatPlayer);
	const int getPlayerSize();
	void removePlayer(int playerID);

	int getEnemies(const int& whatEnemy);
	const int getEnemySize();
	int createEnemy(int type = -1, const std::string &script = "", glm::vec3 pos = glm::vec3(0,0,0), glm::vec3 rot = glm::vec3(0,0,0));

	//we don't have a scene handler here so this is it instead
	void setScriptHandler(ServerScriptHandler* scriptHandler);

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

	void init();
	void update(float dt);

	inline entt::registry& getSceneReg() { return this->reg; }
	inline std::vector<LuaSystem>& getLuaSystems() { return this->luaSystems; }

	void GivePacketInfo(std::vector<sf::Packet>& serverToClient);	

	template <typename I, typename F>
	void addEvent(std::initializer_list<I> ints, std::initializer_list<F> floats)
	{
		//always 0 in "this->serverToClient[0][i]"
		//beacuse it points to the first object in array that doesn't exist
		//so it points at start
		for (int i = 0; i < serverToClient->size(); i++)
		{
			for (auto el : ints)
			{
				this->serverToClient[0][i] << el;
			}
			for (auto el : floats)
			{
				this->serverToClient[0][i] << el;
			}
		}
	}
	template <typename I, typename F>
	void addEvent(std::vector<I> ints, std::vector<F> floats)
	{
		//always 0 in "this->serverToClient[0][i]"
		//beacuse it points to the first object in array that doesn't exist
		//so it points at start
		for (int i = 0; i < serverToClient->size(); i++)
		{
			for (auto el : ints)
			{
				this->serverToClient[0][i] << el;
			}
			for (auto el : floats)
			{
				this->serverToClient[0][i] << el;
			}
		}
	}
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