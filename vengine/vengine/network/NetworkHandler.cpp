#include "pch.h"
#include "NetworkHandler.h"
#include "ServerEngine/Timer.h"
#include <iostream>
#include "../graphics/DebugRenderer.hpp"

void serverMain(bool& shutDownServer, bool& created, NetworkHandler* networkHandler, NetworkScene* game)
{
	Timer serverTime;
	Server server(networkHandler, game);
	bool serverIsDone = false;
	created = true;
	game->setServer(&server);

	while (!shutDownServer && !serverIsDone)
	{
		serverIsDone = server.update(serverTime.getDT());
		serverTime.updateDeltaTime();
	}
	server.disconnect();

	return;
}

void NetworkHandler::createAPlayer(int serverId, const std::string& playerName)
{
	otherPlayers.push_back(std::pair<int, std::string>(0, playerName));
	otherPlayersServerId.push_back(serverId);
}

NetworkHandler::NetworkHandler() 
	: sceneHandler(nullptr)
{
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

void NetworkHandler::setResourceManager(ResourceManager* resourceManager)
{
	this->resourceManger = resourceManager;
}

void NetworkHandler::setMeshes(const std::string& meshName, const int meshID)
{
	this->networkHandlerMeshes.insert(std::pair<std::string, int>(meshName, meshID));
}

void NetworkHandler::handleTCPEventClient(sf::Packet& tcpPacket, int event)
{
}

void NetworkHandler::handleUDPEventClient(sf::Packet& udpPacket, int event)
{
}

void NetworkHandler::handleTCPEventServer(Server* server, int clientID, sf::Packet& tcpPacket, int event)
{
}

void NetworkHandler::handleUDPEventServer(Server* server, int clientID, sf::Packet& tcpPacket, int event)
{
}

void NetworkHandler::onDisconnect(int index)
{
}

void NetworkHandler::createServer(NetworkScene* serverGame)
{
	if (this->hasServer())
	{
		this->shutDownServer = true;
		serverThread->join();
		delete serverThread;
		serverThread = nullptr;
	}

	this->shutDownServer = false;
	this->createdServer = false;
	serverThread = new std::thread(serverMain, std::ref(this->shutDownServer), std::ref(this->createdServer), this, serverGame);

	Timer timer;
	float timeSinceStartCreatingServer = 0;
	while (!this->createdServer && timeSinceStartCreatingServer < waitTimeForServerCreation)
	{
		timeSinceStartCreatingServer += timer.getRealDT();
		timer.updateDeltaTime();
	}
	if (!this->createdServer)
	{
		Log::write("Failed to create server");
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
		this->playerName = name;
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
	if (!client->connect(serverIP))
	{
		delete client;
		client = nullptr;
		return false;
	}
	return true;
}

void NetworkHandler::update()
{
	if (client == nullptr)
	{
		return;
	}

	client->update(Time::getDT());

	int event;
	int h1, h2;
	sf::Packet packet = client->getTCPDataFromServer();
	while (!packet.endOfPacket())
	{
		if (!(packet >> event))
		{
			Log::write("Client: couldn't extract event TCP from server");
			packet.clear();
		}

		if (event == (int)NetworkEvent::CLIENTJOINED)
		{
			std::string playerName;
			packet >> playerName;
			packet >> h1;
			otherPlayers.push_back(std::pair<int, std::string>(-1, playerName));
			otherPlayersServerId.push_back(h1);
		}
		else if (event == (int)NetworkEvent::JUSTJOINED)
		{
			std::string playerName;
			packet >> this->ID; // Own ID in server
			packet >> h1; // Other client amount
			for (int i = 0; i < h1; i++)
			{
				packet >> playerName;
				packet >> h2; // Other client ID
				otherPlayers.push_back(std::pair<int, std::string>(-1, playerName));
				otherPlayersServerId.push_back(h2);
			}
			this->serverStatus = ServerStatus::WAITING;
		}
		else if (event == (int)NetworkEvent::DISCONNECT)
		{
			packet >> h1;
			// We got kicked
			if (h1 == -1)
			{
				this->disconnectClient();
			}
			// Server shut down
			else if (h1 == -2)
			{
				Log::write("Server shut down, disconnecting...");
				this->disconnectClient();
			}
			// Other client got disconnected
			else
			{
				for (int i = 0; i < otherPlayersServerId.size(); i++)
				{
					if (otherPlayersServerId[i] == h1)
					{
						if (otherPlayers.size() < i)
						{
							return;
						}
						sceneHandler->getScene()->removeEntity(otherPlayers[i].first);
						this->onDisconnect(i);
						this->otherPlayers.erase(otherPlayers.begin() + i);
						this->otherPlayersServerId.erase(otherPlayersServerId.begin() + i);
					}
				}
			}
		}
		else if (event == (int)NetworkEvent::GETNAMES)
		{
			packet >> h1;
			Log::write("Getting names from server:");
			for (int i = 0; i < h1; i++)
			{
				std::string playerName;
				packet >> h2 >> playerName;
				Log::write(playerName);
			}
		}
		else if (event == (int)NetworkEvent::START)
		{
			this->serverStatus = ServerStatus::RUNNING;
		}
        else if (event == (int)NetworkEvent::DEBUG_DRAW_BOX)
        {
            glm::vec3 pos;
            glm::vec3 rot;
            glm::vec3 extends;
            packet >> 
				pos.x >> pos.y >> pos.z >> 
				rot.x >> rot.y >> rot.z >> 
				extends.x >> extends.y >> extends.z; 
            this->sceneHandler->getDebugRenderer()->renderBox(pos, rot, extends, glm::vec3(1,0,0));
        }
        else if (event == (int)NetworkEvent::DEBUG_DRAW_SPHERE)
        {
            glm::vec3 pos;
            float radius;
            packet >> pos.x >> pos.y >> pos.z >> radius;
            this->sceneHandler->getDebugRenderer()->renderSphere(pos, radius, glm::vec3(1, 0, 0));
        }
        else if (event == (int)NetworkEvent::DEBUG_DRAW_CYLINDER)
        {
            glm::vec3 pos;
            glm::vec3 rot;
            float height, radius;
            packet >> pos.x >> pos.y >> pos.z >> rot.x >> rot.y >> rot.z >> height >> radius;
            this->sceneHandler->getDebugRenderer()->renderCapsule(pos, rot, height, radius, glm::vec3(1, 0, 0));
        }
        else if (event == (int)NetworkEvent::DEBUG_DRAW_LINE)
        {
            glm::vec3 pos1;
            glm::vec3 pos2;
            pos1 = getVec(packet);
            pos2 = getVec(packet);
            this->sceneHandler->getDebugRenderer()->renderLine(pos1, pos2, glm::vec3(1, 0, 0));
        }
		// Custom event
		else if (event >= (int)NetworkEvent::END)
		{
			this->handleTCPEventClient(packet, event);
		}
	}

	packet = client->getUDPDataFromServer();
	while (!packet.endOfPacket())
	{
		if (!(packet >> event))
		{
			Log::write("Client: couldn't extract UDP event from server");
			packet.clear();
		}
		else if (event == (int)NetworkEvent::DISCONNECT)
		{
			packet >> h1;
			// We got kicked
			if (h1 == -1)
			{
				this->disconnectClient();
			}
			// Server shut down
			else if (h1 == -2)
			{
				Log::write("Server shut down, disconnecting...");
				this->disconnectClient();
			}
			// Other client got disconnected
			else
			{
				for (int i = 0; i < otherPlayersServerId.size(); i++)
				{
					if (otherPlayersServerId[i] == h1)
					{
						if (otherPlayers.size() < i)
						{
							return;
						}
						sceneHandler->getScene()->removeEntity(otherPlayers[i].first);
						this->onDisconnect(i);
						this->otherPlayers.erase(otherPlayers.begin() + i);
						this->otherPlayersServerId.erase(otherPlayersServerId.begin() + i);
					}
				}
			}
		}
		// Custom event
		else if (event >= (int)NetworkEvent::END)
		{
			this->handleUDPEventClient(packet, event);
		}
	}
}

void NetworkHandler::sendDataToServerTCP(sf::Packet packet)
{
	if (client != nullptr)
	{
		client->getTCPPacket().append(packet.getData(), packet.getDataSize());
	}
}

void NetworkHandler::sendDataToServerUDP(sf::Packet packet)
{
	if (client != nullptr)
	{
		client->getUDPPacket().append(packet.getData(), packet.getDataSize());
	}
}

void NetworkHandler::setPlayerNetworkHandler(int playerID)
{
	this->player = playerID;
}

const std::string& NetworkHandler::getClientName()
{
	return this->playerName;
}

const bool NetworkHandler::isConnected()
{
	if (this->client != nullptr)
	{
		return this->client->isConnected();
	}
	return false;
}
const std::string& NetworkHandler::getClientName()
{
	return this->playerName;
}
void NetworkHandler::getLuaData(std::vector<int>& ints, std::vector<float>& floats)
{
	//ints = this->lua_ints;
	//floats = this->lua_floats;
}

const bool NetworkHandler::hasServer()
{
	return serverThread != nullptr;
}

std::vector<std::pair<int, std::string>> NetworkHandler::getPlayers()
{
	return this->otherPlayers;
}

void NetworkHandler::createPlayers()
{
	for (int i = 0; i < otherPlayers.size(); i++)
	{
		otherPlayers[i].first = sceneHandler->getScene()->createEntity();
		
		if (this->networkHandlerMeshes.find("PlayerMesh") == this->networkHandlerMeshes.end())
		{
			sceneHandler->getScene()->setComponent<MeshComponent>(otherPlayers[i].first);
		}
		else
		{
			sceneHandler->getScene()->setComponent<MeshComponent>(otherPlayers[i].first, this->networkHandlerMeshes.find("PlayerMesh")->second
			);
		}
	}
}

glm::vec3 NetworkHandler::getVec(sf::Packet& packet)
{
	glm::vec3 vec;
	packet >> vec.x >> vec.y >> vec.z;
	return vec;
}

void NetworkHandler::sendVec(sf::Packet& packet, const glm::vec3& vec)
{
	packet << vec.x << vec.y << vec.z;
}

void NetworkHandler::disconnectClient()
{
	if (client != nullptr)
	{
		if (client->isConnected())
		{
			client->disconnect();
			this->otherPlayers.clear();
			this->otherPlayersServerId.clear();
			this->serverStatus = ServerStatus::DISCONNECTED;
		}
	}
}