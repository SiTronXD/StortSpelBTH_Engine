#include "NetworkHandler.h"
#include <iostream>
#include "Timer.h"

void serverMain(bool& shutDownServer, ServerGame* game)
{
	Timer serverTime;
	Server server(game);
	bool serverIsDone = false;

	while (!shutDownServer && !serverIsDone)
	{
		serverIsDone = server.update(serverTime.getDT());
		serverTime.updateDeltaTime();
	}
	return;
}

NetworkHandler::NetworkHandler()
{
	fx = fy = fz = fa = fb = fc = 0.f;
	ix = iy = iz = ia = ib = ic = 0;
	shutDownServer = false;
	client = nullptr;
	serverThread = nullptr;
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

void NetworkHandler::createServer(ServerGame* serverGame)
{
	serverThread =
	    new std::thread(serverMain, std::ref(this->shutDownServer), serverGame);
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
						sceneHandler->getScene()->removeEntity(otherPlayers[i]);
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
			otherPlayers.push_back(sceneHandler->getScene()->createEntity());
			sceneHandler->getScene()->setComponent<MeshComponent>(
			    otherPlayers[otherPlayers.size() - 1]
			);
			Transform& transform =
			    sceneHandler->getScene()->getComponent<Transform>(
			        otherPlayers[otherPlayers.size() - 1]
			    );

			transform.scale = glm::vec3(10.0f, 5.0f, 5.0f);
			transform.position =
			    glm::vec3(-30.0f + (otherPlayers.size() * 50), 0.0f, 30.0f);
		}
		else if (gameEvent == GameEvents::ID)
		{
			cTCPP >> this->ID;  //id of this player
			cTCPP >> ix;        //nr of players in this game
			std::cout << "players in this game: " << ix << std::endl;
			for (int i = 0; i < ix; i++)
			{
				otherPlayers.push_back(sceneHandler->getScene()->createEntity()
				);
				sceneHandler->getScene()->setComponent<MeshComponent>(
				    otherPlayers[otherPlayers.size() - 1]
				);
				Transform& transform =
				    sceneHandler->getScene()->getComponent<Transform>(
				        otherPlayers[otherPlayers.size() - 1]
				    );

				transform.scale = glm::vec3(10.0f, 5.0f, 5.0f);
				transform.position =
				    glm::vec3(-30.0f + (otherPlayers.size() * 50), 0.0f, 30.0f);
			}
		}
		else if (gameEvent == GameEvents::SpawnEnemy)
		{
			//ix = what type of enemy
			cTCPP >> ix;
			//should we really create a new entity everytime?
			iy = sceneHandler->getScene()->createEntity();
			monsters.push_back(iy);

			sceneHandler->getScene()->setComponent<MeshComponent>(iy);

			cTCPP >> fx >> fy >> fz;
			Transform& transform =
			    sceneHandler->getScene()->getComponent<Transform>(iy);
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
				Transform& transform =
				    sceneHandler->getScene()->getComponent<Transform>(iy);
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

				Transform& transform =
				    sceneHandler->getScene()->getComponent<Transform>(
				        otherPlayers[i]
				    );
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
				monsters.resize(ix);
			}
			if (monsters.size() < ix)
			{
				//create entity
			}

			for (int i = 0; i < ix; i++)
			{
				//fxyz position, fabc rotation
				cUDPP >> fx >> fy >> fz >> fa >> fb >> fc;
				Transform& transform =
				    sceneHandler->getScene()->getComponent<Transform>(
				        monsters[i]
				    );
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

void NetworkHandler::sendUDPDataToClient(
    const glm::vec3& pos, const glm::vec3& rot
)
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
	//TODO : send polygons to the server
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

void NetworkHandler::disconnectClient()
{
	client->disconnect();
	if (client != nullptr)
	{
		delete client;
		client = nullptr;
	}
}
