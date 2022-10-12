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

void ServerGame::addPolygon(NavMesh::Polygon& polygon) {
    pf.addPolygon(polygon);
}

void ServerGame::addPolygon(const std::vector<float>& polygon) {
    NavMesh::Polygon p;
    for (size_t i = 0; i < polygon.size(); i += 2) {
        p.AddPoint(polygon[i], polygon[i + 1]);
    }
    pf.addPolygon(p);
}

void ServerGame::removeAllPolygons() {
    pf.removeAllPolygons();
}
const int ServerGame::getSeed()
{
    return this->seed;
}
