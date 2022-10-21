#include "ServerGame.h"

ServerGameMode::ServerGameMode()
{
#ifndef NDEBUG
#define SERVERSEED 69
	srand(SERVERSEED);
#else
	srand((unsigned)time(NULL));
#endif
	this->seed = rand();
}

ServerGameMode::~ServerGameMode() {}

void ServerGameMode::createPlayer()
{
	this->players.push_back(ServerEntity());
}

std::vector<ServerEntity>& ServerGameMode::getServerEntities()
{
	return this->serverEntities;
}

std::vector<ServerEntity>& ServerGameMode::getServerPlayers()
{
	return this->players;
}

void ServerGameMode::GivePacketInfo(std::vector<sf::Packet>& serverToClient)
{
	this->serverToClient = &serverToClient;
}

void ServerGameMode::spawnEnemy(glm::vec3 pos)
{
	//this->
}

void ServerGameMode::setScene(NetworkScene* scene)
{
	this->scene = scene;
}

void ServerGameMode::setScriptHandler(ServerScriptHandler* scriptHandler)
{
	this->scriptHandler = scriptHandler;
}

void ServerGameMode::addPolygon(NavMesh::Polygon& polygon)
{
	pf.addPolygon(polygon);
}

void ServerGameMode::addPolygon(const std::vector<float>& polygon)
{
	NavMesh::Polygon p;
	for (size_t i = 0; i < polygon.size(); i += 2)
	{
		p.AddPoint(polygon[i], polygon[i + 1]);
	}
	pf.addPolygon(p);
}

void ServerGameMode::removeAllPolygons()
{
	pf.removeAllPolygons();
}
const int ServerGameMode::getSeed()
{
	return this->seed;
}
