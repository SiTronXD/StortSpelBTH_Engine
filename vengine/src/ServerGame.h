#pragma once
#include <vector>
#include "glm/vec3.hpp"
#include "SFML/Network.hpp"
#include "NetworkEnumAndDefines.h"

struct ServerEntity {
	glm::vec3 position;
	glm::vec3 rotation;
	int type;
};

class ServerGame {
public:
	ServerGame();
	void update(float dt);
	virtual ~ServerGame();
	void createPlayer();
	std::vector<ServerEntity>& getServerEntities();
	std::vector<ServerEntity>& getServerPlayers();
	void GivePacketInfo(std::vector<sf::Packet> &serverToClient);
	
private:
	
	std::vector<ServerEntity> serverEntities;
	std::vector<ServerEntity> players;
	std::vector<sf::Packet> serverToClient;

	template<typename I, typename F>
	void addEvent(std::initializer_list<I> ints, std::initializer_list<F> floats)
	{
		for (int i = 0; i < serverToClient.size(); i++) {
			for (auto el : ints) {
				this->serverToClient[i] << el;
			}
			for (auto el : floats) {
				this->serverToClient[i] << el;
			}
		}
	}

	//DEBUG
	bool Went_in = false;
};