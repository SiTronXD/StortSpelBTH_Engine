#include "ServerGame.h"

ServerGame::ServerGame()
{
}

void ServerGame::update(float dt)
{
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
