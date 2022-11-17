#include "pch.h"
#include "NetworkScene.h"

NetworkScene::NetworkScene() {}

NetworkScene::~NetworkScene() {}

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
	enemies.clear();
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

int NetworkScene::getPlayer(const int& whatPlayer)
{
	return this->players[whatPlayer];
}

const int NetworkScene::getPlayerSize()
{
	return (int)this->players.size();
}

int NetworkScene::getEnemies(const int& whatPlayer)
{
	return this->enemies[whatPlayer].first;
}

const int NetworkScene::getEnemySize()
{
	return (int)this->enemies.size();
}

int NetworkScene::createEnemy(int type, const std::string& script, glm::vec3 pos, glm::vec3 rot)
{
	int e = this->createEntity();
	this->setComponent<Transform>(e);
	this->getComponent<Transform>(e).position = pos;
	this->getComponent<Transform>(e).rotation = rot;
	if (script != "")
	{
		this->setScriptComponent(e, script);
	}
	this->enemies.push_back(std::pair<int, int>(e, type));
	//this->addEvent({(int)GameEvents::SpawnEnemy, type}, {pos.x, pos.y, pos.z, rot.x, rot.y, rot.z});
	return e;
}

int NetworkScene::createPlayer()
{
	int e = this->createEntity();
	this->setComponent<Transform>(e);
	this->players.push_back(e);
	return e;
}

void NetworkScene::removePlayer(int playerID)
{
	players.erase(players.begin() + playerID);
}

void NetworkScene::givePacketInfo(std::vector<sf::Packet>& serverToClient)
{
	this->serverToClient = &serverToClient;
}
