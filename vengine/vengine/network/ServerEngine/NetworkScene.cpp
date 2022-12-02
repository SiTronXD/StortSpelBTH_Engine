#include "pch.h"
#include "NetworkScene.h"
#include "Server.h"

NetworkScene::NetworkScene() : serverToClientTCP(nullptr), serverToClientUDP(nullptr), server(nullptr) {
    this->sceneType = SceneType::NetworkScene;
}

NetworkScene::~NetworkScene() {}

void NetworkScene::setServer(Server* server)
{
	this->server = server;
}

Server* NetworkScene::getServer()
{
    return this->server;
}

PathFindingManager& NetworkScene::getPathFinder()
{
	return this->pf;
}

void NetworkScene::addPolygon(NavMesh::Polygon& polygon)
{
	pf.addPolygon(polygon);
}

void NetworkScene::addPolygon(const std::vector<float>& polygon)
{
	NavMesh::Polygon p;
	for (size_t i = 0; i < polygon.size(); i += 2)
	{
		p.AddPoint(polygon[i], polygon[i + 1]);
	}
	pf.addPolygon(p);
}

void NetworkScene::removeAllPolygons()
{
	pf.removeAllPolygons();
}

void NetworkScene::removeAllEntitys()
{

	this->getSceneReg().clear();
	players.clear();
}

void NetworkScene::updateSystems(float dt)
{
	for (auto it = this->systems.begin(); it != this->systems.end();)
	{
		if ((*it)->update(this->getSceneReg(), dt))
		{
			delete (*it);
			it = this->systems.erase(it);
		}
		else
		{
			it++;
		}
	}
}

void NetworkScene::init() {}

void NetworkScene::update() {}

void NetworkScene::update(float dt)
{
	this->updateSystems(dt);
	this->getScriptHandler()->updateSystems(this->luaSystems);

	auto view = this->getSceneReg().view<Transform>(entt::exclude<Inactive>);
	auto func = [](Transform& transform)
	{
		transform.updateMatrix();
	};
	view.each(func);
}

void NetworkScene::onDisconnect(int index)
{
}

int NetworkScene::getPlayer(const int& whatPlayer)
{
	return this->players[whatPlayer];
}

std::vector<int>* NetworkScene::getPlayers()
{
	return &this->players;
}

int NetworkScene::getNearestPlayer(const int& ent)
{
	int returnIndex = -1;
	float nearestLenght = glm::length(this->getComponent<Transform>(ent).position - this->getComponent<Transform>(players[0]).position);
	for (int i = 0; i < this->players.size(); i++)
	{
		float nnl = glm::length(this->getComponent<Transform>(ent).position - this->getComponent<Transform>(players[i]).position);

		if (nnl < nearestLenght)
		{
			nearestLenght = nnl;
			returnIndex = i;
		}
	}
	return returnIndex;
}

bool NetworkScene::isAPlayer(const int& entity)
{
	for (int i = 0; i < this->players.size(); i++)
	{
		if (players[i] == entity)
		{
			return true;
		}
	}
	return false;
}

const int NetworkScene::getPlayerSize()
{
	return (int)this->players.size();
}

int NetworkScene::createPlayer()
{
	int e = this->createEntity();
	this->setComponent<Transform>(e);
	this->setComponent<Collider>(e, Collider::createCapsule(2, 11, glm::vec3(0, 7.3, 0)));
	this->players.push_back(e);
	return e;
}

void NetworkScene::removePlayer(int playerID)
{
	players.erase(players.begin() + playerID);
}

void NetworkScene::givePacketInfo(std::vector<sf::Packet>* serverToClientTCP)
{
	this->serverToClientTCP = serverToClientTCP;
}

void NetworkScene::givePacketInfoUdp(std::vector<sf::Packet>* serverToClientUDP)
{
    this->serverToClientUDP = serverToClientUDP;
}
