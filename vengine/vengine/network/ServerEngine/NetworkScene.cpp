#include "NetworkScene.h"

ServerScriptHandler* NetworkScene::getScriptHandler()
{
	return this->scriptHandler;
}

NetworkScene::NetworkScene()
{
	this->reg.clear();
}

NetworkScene::~NetworkScene()
{
	for (size_t i = 0; i < this->systems.size(); ++i)
	{
		delete this->systems[i];
	}
	this->reg.clear();
	this->systems.clear();
	this->luaSystems.clear();
}

void NetworkScene::setScriptHandler(ServerScriptHandler* scriptHandler)
{
	this->scriptHandler = scriptHandler;
}

void NetworkScene::createSystem(std::string& path)
{
	this->luaSystems.push_back(LuaSystem{path, -1});
}

void NetworkScene::setScriptComponent(Entity entity, std::string path)
{
	this->getScriptHandler()->setScriptComponent(entity, path);
}

void NetworkScene::updateSystems(float dt)
{
	for (auto it = this->systems.begin(); it != this->systems.end();)
	{
		//doesn't dare to use Time::getDT()
		if ((*it)->update(this->reg, dt))
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

int NetworkScene::getEntityCount() const
{
	return (int)this->reg.alive();
}

bool NetworkScene::entityValid(Entity entity) const
{
	return this->reg.valid((entt::entity)entity);
}

Entity NetworkScene::createEntity()
{
	int entity = (int)this->reg.create();
	this->setComponent<Transform>(entity);
	return entity;
}

bool NetworkScene::removeEntity(Entity entity)
{
	bool valid = this->entityValid(entity);
	if (valid)
	{
		this->reg.destroy((entt::entity)entity);
	}

	//TODO : check if an enemy was this entity?
	
	return valid;
}

void NetworkScene::removeAllEntitys()
{
	this->reg.clear();
	players.clear();
	enemies.clear();
}

void NetworkScene::setActive(Entity entity)
{
	this->removeComponent<Inactive>(entity);
}

void NetworkScene::setInactive(Entity entity)
{
	this->setComponent<Inactive>(entity);
}

bool NetworkScene::isActive(Entity entity)
{
	return !this->hasComponents<Inactive>(entity);
}

void NetworkScene::init() {}

void NetworkScene::update(float dt)
{
	this->updateSystems(dt);
	this->scriptHandler->updateSystems(this->luaSystems);
}

int NetworkScene::getPlayer(const int& whatPlayer)
{
	return this->players[whatPlayer];
}

const int NetworkScene::getPlayerSize()
{
	return this->players.size();
}

int NetworkScene::getEnemies(const int& whatPlayer)
{
	return this->enemies[whatPlayer].first;
}

const int NetworkScene::getEnemySize()
{
	return this->enemies.size();
}

int NetworkScene::createEnemy(int type, std::string script, glm::vec3 pos, glm::vec3 rot)
{
	int e = this->createEntity();
	this->setComponent<Transform>(e);
	this->getComponent<Transform>(e).position = pos;
	this->getComponent<Transform>(e).rotation = rot;
	if (script != "")
	{
		this->setScriptComponent(e, script);
	}
	this->enemies.push_back(std::pair<int,int>(e, type));
	this->addEvent({(int)GameEvents::SpawnEnemy, type}, {pos.x, pos.y, pos.z, rot.x, rot.y, rot.z});
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

void NetworkScene::GivePacketInfo(std::vector<sf::Packet>& serverToClient)
{
	this->serverToClient = &serverToClient;
}
