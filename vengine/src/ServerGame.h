#pragma once
#include <vector>
#include "glm/vec3.hpp"

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
private:
	std::vector<ServerEntity> serverEntities;
	std::vector<ServerEntity> players;
};