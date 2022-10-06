#include "ServerGame.h"

ServerGame::ServerGame()
{
#ifndef NDEBUG
    #define SERVERSEED 69
    srand(SERVERSEED);
#else
    srand((unsigned)time(NULL));
#endif
    this->seed = rand();
}

ServerGame::~ServerGame() {}

void ServerGame::createPlayer()
{
    this->players.push_back(ServerEntity());
}

std::vector<ServerEntity>& ServerGame::getServerEntities()
{
    return this->serverEntities;
}

std::vector<ServerEntity>& ServerGame::getServerPlayers()
{
    return this->players;
}

void ServerGame::GivePacketInfo(std::vector<sf::Packet>& serverToClient)
{
    this->serverToClient = &serverToClient;
}

const int ServerGame::getSeed()
{
    return this->seed;
}
