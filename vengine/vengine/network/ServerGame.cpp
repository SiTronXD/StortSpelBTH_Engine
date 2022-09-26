#include "ServerGame.h"

ServerGame::ServerGame()
{
}

void ServerGame::update(float dt)
{
    if (players[0].position.z > 50 && !Went_in) {
        Went_in = true;
        serverEntities.push_back(ServerEntity{glm::vec3(0,0,70), glm::vec3(0,0,0), 1});
        addEvent({ (int)GameEvents::SpawnEnemy, 1 }, { 0.f, 0.f, 70.f });
    }
    for (int i = 0; i < serverEntities.size(); i++) {
        serverEntities[i].rotation.x += dt * 5 * (i + 1);
        if (serverEntities[i].rotation.x > 360) {
            serverEntities[i].rotation.x = 0;
        }
    }
}

ServerGame::~ServerGame()
{
}

void ServerGame::createPlayer()
{
    players.push_back(ServerEntity());
}

std::vector<ServerEntity>& ServerGame::getServerEntities()
{
    return this->serverEntities;
}

std::vector<ServerEntity>& ServerGame::getServerPlayers()
{
    return players;
}

void ServerGame::GivePacketInfo(std::vector<sf::Packet>& serverToClient)
{
    this->serverToClient = &serverToClient;
}
