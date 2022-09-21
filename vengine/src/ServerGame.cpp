#include "ServerGame.h"

ServerGame::ServerGame()
{
}

void ServerGame::update(float dt)
{
    if (players[0].position.z > 50 && !Went_in) {
        Went_in = true;
        serverEntities.push_back(ServerEntity{glm::vec3(0,0,20), glm::vec3(0,0,0), 1});
        for (int i = 0; i < serverToClient.size(); i++) {
            addEvent({ (int)GameEvents::SpawnEnemy, 1 }, { 0.f, 0.f, 20.f });
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
    this->serverToClient = serverToClient;
}
