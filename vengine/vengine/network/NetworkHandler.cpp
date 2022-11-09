#include "pch.h"
#include "NetworkHandler.h"
#include "ServerEngine/Timer.h"
#include <iostream>


void serverMain(bool& shutDownServer, bool& created, NetworkScene* game)
{
	Timer serverTime;
	Server server(game);
	bool serverIsDone = false;
	created = true;

	while (!shutDownServer && !serverIsDone)
	{
		serverIsDone = server.update(serverTime.getDT());
		serverTime.updateDeltaTime();
	}

	return;
}

void NetworkHandler::createAPlayer(int serverId, const std::string& playerName) {
	otherPlayers.push_back(std::pair<int, std::string>(sceneHandler->getScene()->createEntity(), playerName));
	otherPlayersServerId.push_back(serverId);
	//TODO : get player mesh
	sceneHandler->getScene()->setComponent<MeshComponent>(otherPlayers[otherPlayers.size() - 1].first);
}

NetworkHandler::NetworkHandler() : sceneHandler(nullptr)
{
	this->fx = fy = fz = fa = fb = fc = 0.f;
	this->ix = iy = iz = ia = ib = ic = 0;
	this->shutDownServer = false;
	this->client = nullptr;
	this->serverThread = nullptr;
	this->player = -1;
}

NetworkHandler::~NetworkHandler()
{
	shutDownServer = true;
	if (serverThread != nullptr)
	{
		serverThread->join();
		delete serverThread;
		serverThread = nullptr;
	}
	if (client != nullptr)
	{
		delete client;
		client = nullptr;
	}
}

void NetworkHandler::setSceneHandler(SceneHandler* sceneHandler)
{
	this->sceneHandler = sceneHandler;
}

void NetworkHandler::setResourceManager(ResourceManager* resourceManager) {
	this->resourceManger = resourceManager;
	monsterResId[0] = this->resourceManger->addMesh("assets/models/Swarm_model.obj");
	monsterResId[1] = this->resourceManger->addMesh("assets/models/Amogus/source/1.fbx");
	monsterResId[2] = this->resourceManger->addMesh("assets/models/Amogus/source/1.fbx");
}

void NetworkHandler::createServer(NetworkScene* serverGame)
{
	if (serverThread == nullptr)
	{
		serverThread = new std::thread(serverMain, std::ref(this->shutDownServer), std::ref(this->createdServer), serverGame);

		Timer timer;
		float timeSinceStartCreatingServer = 0;
		while (!this->createdServer && timeSinceStartCreatingServer < waitTimeForServerCreation)
		{
			timeSinceStartCreatingServer += timer.getRealDT();
			timer.updateDeltaTime();
		}
		if (!this->createdServer)
		{
			std::cout << "failed to create server" << std::endl;
		}
	}
	else
	{  //shut down server and create it again
		this->shutDownServer = true;
		serverThread->join();
		delete serverThread;
		serverThread = nullptr;
		this->shutDownServer = false;
		serverThread = new std::thread(serverMain, std::ref(this->shutDownServer), std::ref(this->createdServer), serverGame);

		Timer timer;
		float timeSinceStartCreatingServer = 0;
		while (!this->createdServer && timeSinceStartCreatingServer < waitTimeForServerCreation)
		{
			timeSinceStartCreatingServer += timer.getRealDT();
			timer.updateDeltaTime();
		}
		if (!this->createdServer)
		{
			std::cout << "failed to create server" << std::endl;
		}
	}
}

void NetworkHandler::deleteServer()
{
	shutDownServer = true;
	if (serverThread != nullptr)
	{
		serverThread->join();
		delete serverThread;
		serverThread = nullptr;
	}

	if (client != nullptr)
	{
		delete client;
		client = nullptr;
	}
}

Client* NetworkHandler::createClient(const std::string& name)
{
	if (client == nullptr)
	{
		client = new Client(name);
	}
	return client;
}

Client* NetworkHandler::getClient()
{
	return client;
}

bool NetworkHandler::connectClientToThis()
{
	return this->connectClient(sf::IpAddress::getLocalAddress().toString());
}

bool NetworkHandler::connectClient(const std::string& serverIP)
{
	if (client == nullptr)
	{
		std::cout << "client doesn't exist" << std::endl;
		return false;
	}
	return client->connect(serverIP);
}

void NetworkHandler::updateNetwork()
{
	if (client == nullptr)
	{
		return;
	}

	if (player != -1)
	{
		this->sendUDPDataToClient(
		    this->sceneHandler->getScene()->getComponent<Transform>(this->player).position,
		    this->sceneHandler->getScene()->getComponent<Transform>(this->player).rotation
		);
	}

	client->update(Time::getDT());
	//tcp
	sf::Packet cTCPP = client->getTCPDataFromServer();
	int gameEvent;
	while (!cTCPP.endOfPacket())
	{
		cTCPP >> gameEvent;
		if (gameEvent == GameEvents::DISCONNECT)
		{
			cTCPP >> ix;
			if (ix == -1)
			{  //we got kicked
				this->disconnectClient();
				//sceneHandler->setScene();
			}
			else
			{
				for (int i = 0; i < otherPlayersServerId.size(); i++)
				{
					if (otherPlayersServerId[i] == ix)
					{
						if (otherPlayers.size() < i)
						{
							return;
						}
						sceneHandler->getScene()->removeEntity(otherPlayers[i].first);
						this->otherPlayers.erase(otherPlayers.begin() + i);
						this->otherPlayersServerId.erase(otherPlayersServerId.begin() + i);
					}
				}
			}
		}
		else if (gameEvent == GameEvents::START)
		{
			client->starting();
		}
		else if (gameEvent == GameEvents::PlayerJoined)
		{
			std::string playerName;
			cTCPP >> playerName;
			cTCPP >> ix;
			this->createAPlayer(ix, playerName);
		}
		else if (gameEvent == GameEvents::SpawnEnemy)
		{
			//ix = what type of enemy
			cTCPP >> ix;
			
			//should we really create a new entity everytime?
			iy = sceneHandler->getScene()->createEntity();
			std::cout << "spawn enemy" << iy << std::endl;
			monsters.push_back(iy);

			sceneHandler->getScene()->setComponent<MeshComponent>(iy, monsterResId[ix]);

			cTCPP >> fx >> fy >> fz;
			Transform& transform = sceneHandler->getScene()->getComponent<Transform>(iy);
			transform.position = glm::vec3(fx, fy, fz);
		}
		else if (gameEvent == GameEvents::SpawnEnemies)
		{
			//ix = what type of enemy, iy how many enemies
			cTCPP >> ix >> iy;

			for (int i = 0; i < iy; i++)
			{
				iz = sceneHandler->getScene()->createEntity();
				sceneHandler->getScene()->setComponent<MeshComponent>(iz);

				//ix = what type of enemy
				cTCPP >> fx >> fy >> fz;
				Transform& transform = sceneHandler->getScene()->getComponent<Transform>(iy);
				transform.position = glm::vec3(fx, fy, fz);
			}
		}
		else if (gameEvent == GameEvents::Explosion)
		{
			//don't know how this should be implemented right now
		}
		else if (gameEvent == GameEvents::MonsterDied)
		{
			//don't know how this should be implemented right now
		}
		else if (gameEvent == GameEvents::PlayerShoot)
		{
			//don't know how this should be implemented right now
		}
		else if (gameEvent == GameEvents::GAMEDATA)
		{
			cTCPP >> ix;  //nrOfPlayers;

			for (int i = 0; i < ix; i++)
			{
				cTCPP >> iz;
				if (iz != this->ID)
				{
					this->otherPlayersServerId.push_back(iz);
				}
			}

			cTCPP >> this->seed;  //seed nr;
		}
		else if (gameEvent == GameEvents::PlayerDied)
		{
			//don't know how this should be implemented right now
		}
		else if (gameEvent == GameEvents::GetPlayerNames)
		{
			cTCPP >> ix;
			while (ix > otherPlayers.size())
			{
				std::string playerName;
				cTCPP >> iy >> playerName;
				createAPlayer(iy, playerName);
			}
		}
	}
	sf::Packet cUDPP = client->getUDPDataFromServer();
	while (!cUDPP.endOfPacket())
	{
		cUDPP >> gameEvent;
		if (gameEvent == GameEvents::UpdatePlayerPos)
		{
			//ix = amount of players
			cUDPP >> ix;
			for (int i = 0; i < ix; i++)
			{
				//fxyz position, fabc rotation
				cUDPP >> fx >> fy >> fz >> fa >> fb >> fc;

				if (otherPlayers.size() < i)
				{
					//create entity
				}

				Transform& transform = sceneHandler->getScene()->getComponent<Transform>(otherPlayers[i].first);
				transform.position = glm::vec3(fx, fy, fz);
				transform.rotation = glm::vec3(fa, fb, fc);
			}
		}
		else if (gameEvent == GameEvents::UpdateMonsterPos)
		{
			//ix number of monsters
			cUDPP >> ix;
			if (monsters.size() < ix)
			{
				std::cout << "updating size of monster" << std::endl;
				monsters.reserve(ix);
				for (int i = monsters.size(); i < ix; i++)
				{
					iy = sceneHandler->getScene()->createEntity();
					monsters.push_back(iy);

					sceneHandler->getScene()->setComponent<MeshComponent>(iy);
				}
			}

			for (int i = 0; i < ix; i++)
			{
				//fxyz position, fabc rotation
				cUDPP >> fx >> fy >> fz >> fa >> fb >> fc;
				Transform& transform = sceneHandler->getScene()->getComponent<Transform>(monsters[i]);
				transform.position = glm::vec3(fx, fy, fz);
				transform.rotation = glm::vec3(fa, fb, fc);
			}
		}
	}
}

void NetworkHandler::sendTCPDataToClient(TCPPacketEvent tcpP)
{
	if (client != nullptr)
	{
		client->sendTCPEvent(tcpP);
	}
}

void NetworkHandler::setPlayerNetworkHandler(int playerID)
{
	this->player = playerID;
}

void NetworkHandler::sendUDPDataToClient(const glm::vec3& pos, const glm::vec3& rot)
{
	if (client != nullptr)
	{
		client->sendUDPEvent(GameEvents::UpdatePlayerPos, pos, rot);
	}
}

int NetworkHandler::getServerSeed()
{
	return seed;
}
void NetworkHandler::sendAIPolygons(std::vector<glm::vec2> points)
{
	TCPPacketEvent polygonEvent;

	polygonEvent.gameEvent = GameEvents::POLYGON_DATA;  //change this
	polygonEvent.floats.reserve(points.size() * 2);
	polygonEvent.ints[0] = (int)points.size();
	for (int i = 0; i < points.size(); i++)
	{
		polygonEvent.floats.push_back((float)points[i].x);
		polygonEvent.floats.push_back((float)points[i].y);
	}

	polygonEvent.nrOfInts = 1;

	client->sendTCPEvent(polygonEvent);
}

void NetworkHandler::getLuaData(std::vector<int>& ints, std::vector<float>& floats)
{
	ints = this->lua_ints;
	floats = this->lua_floats;
}

const bool NetworkHandler::hasServer()
{
	return serverThread != nullptr;
}

const std::vector<std::pair<int, std::string>> NetworkHandler::getPlayers() 
{
	return this->otherPlayers;
}

void NetworkHandler::disconnectClient()
{
	client->disconnect();
	if (client != nullptr)
	{
		delete client;
		client = nullptr;
	}
}
