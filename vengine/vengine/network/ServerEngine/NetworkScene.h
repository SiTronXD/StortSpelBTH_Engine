#pragma once
#include "../../application/Scene.hpp"
#include "../../ai/PathFinding.h"
#include "../NetworkEnumAndDefines.h"
#include <SFML/Network.hpp>

class NetworkScene : public Scene
{
  private:
	std::vector<sf::Packet>* serverToClient;

protected:
	//id, type
	std::vector<std::pair<int, int>> enemies;
	std::vector<int> players;
	PathFindingManager pf;

  public:
	NetworkScene();
	virtual ~NetworkScene();
	
	//AI things
	PathFindingManager& getPathFinder();
	void addPolygon(NavMesh::Polygon& polygon);
	void addPolygon(const std::vector<float>& polygon);
	void removeAllPolygons();

	/////////////custom function for server only/////////////
	int createPlayer();
	int getPlayer(const int& whatPlayer);
	const int getPlayerSize();
	void removePlayer(int playerID);

	int getEnemies(const int& whatEnemy);
	const int getEnemySize();
	int createEnemy(int type = -1, const std::string& script = "", glm::vec3 pos = glm::vec3(0, 0, 0), glm::vec3 rot = glm::vec3(0, 0, 0));
	/////////////////////////////////////////////////////////
	
	//Time::getDT() doesn't exist so must do extra here
	void removeAllEntitys();
	void updateSystems(float dt);

	virtual void init();
	virtual void update();
	virtual void update(float dt);

	void givePacketInfo(std::vector<sf::Packet>& serverToClient);

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
	template <typename I>
	void addEvent(std::initializer_list<I> ints)
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