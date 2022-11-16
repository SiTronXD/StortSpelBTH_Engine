#include "pch.h"
#include "NetworkHandler.h"
#include "ServerEngine/Timer.h"
#include <iostream>

void serverMain(bool& shutDownServer, bool& created, NetworkHandler* networkHandler, NetworkScene* game)
{
	Timer serverTime;
	Server server(networkHandler, game);
	bool serverIsDone = false;
	created = true;

	while (!shutDownServer && !serverIsDone)
	{
		serverIsDone = server.update(serverTime.getDT());
		serverTime.updateDeltaTime();
	}

	return;
}

void NetworkHandler::createAPlayer(int serverId, const std::string& playerName)
{
	otherPlayers.push_back(std::pair<int, std::string>(sceneHandler->getScene()->createEntity(), playerName));
	otherPlayersServerId.push_back(serverId);
	//TODO : get player mesh
	if (this->networkHandlerMeshes.find("PlayerMesh") == this->networkHandlerMeshes.end())
	{
		sceneHandler->getScene()->setComponent<MeshComponent>(otherPlayers[otherPlayers.size() - 1].first);
	}
	else
	{
		sceneHandler->getScene()->setComponent<MeshComponent>(
			otherPlayers[otherPlayers.size() - 1].first, 
			this->networkHandlerMeshes.find("PlayerMesh")->second
			);
	}
}

NetworkHandler::NetworkHandler() 
	: sceneHandler(nullptr)
{
	this->fx = fy = fz = fa = fb = fc = 0.f;
	this->ix = iy = iz = ia = ib = ic = 0;
	this->shutDownServer = false;
	this->client = nullptr;
	this->serverThread = nullptr;
	this->player = -1;
	this->seed = -1;
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

	//if (serverThread == nullptr)
	//{
	//	this->shutDownServer = false;
	//	this->createdServer = false;
	//	serverThread = new std::thread(serverMain, std::ref(this->shutDownServer), std::ref(this->createdServer), serverGame);

	//	Timer timer;
	//	float timeSinceStartCreatingServer = 0;
	//	while (!this->createdServer && timeSinceStartCreatingServer < waitTimeForServerCreation)
	//	{
	//		timeSinceStartCreatingServer += timer.getRealDT();
	//		timer.updateDeltaTime();
	//	}
	//	if (!this->createdServer)
	//	{
	//		std::cout << "failed to create server" << std::endl;
	//	}
	//}
	//else
	//{  //shut down server and create it again
	//	this->shutDownServer = true;
	//	serverThread->join();
	//	delete serverThread;
	//	serverThread = nullptr;
	//	this->shutDownServer = false;
	//	this->createdServer = false;
	//	serverThread = new std::thread(serverMain, std::ref(this->shutDownServer), std::ref(this->createdServer), serverGame);

	//	Timer timer;
	//	float timeSinceStartCreatingServer = 0;
	//	while (!this->createdServer && timeSinceStartCreatingServer < waitTimeForServerCreation)
	//	{
	//		timeSinceStartCreatingServer += timer.getRealDT();
	//		timer.updateDeltaTime();
	//	}
	//	if (!this->createdServer)
	//	{
	//		std::cout << "failed to create server" << std::endl;
	//	}
	//}
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

	/*if (player != -1)
	{
		this->sendUDPDataToClient(
		    this->sceneHandler->getScene()->getComponent<Transform>(this->player).position,
		    this->sceneHandler->getScene()->getComponent<Transform>(this->player).rotation
		);
	}*/

	client->update(Time::getDT());

	int event;
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
			packet >> ix;
			this->createAPlayer(ix, playerName);
		}
		else if (event == (int)NetworkEvent::DISCONNECT)
		{
			packet >> ix;
			// We got kicked
			if (ix == -1)
			{
				this->disconnectClient();
			}
			// Other client got disconnected
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
		else if (event == (int)GameEvents::START)
		{
			this->client->starting();
		}
		// Custom event
		else if (event >= (int)NetworkEvent::END)
		{
			this->handleTCPEventClient(packet, event);
		}
	}

	packet = client->getUDPDataFromServer();
	//Log::write(std::to_string(client->getUDPDataFromServer().getDataSize()));
	while (!packet.endOfPacket())
	{
		if (!(packet >> event))
		{
			Log::write("Client: couldn't extract UDP event from server");
			packet.clear();
		}
		//if (gameEvent == (int)NetworkEvent::DISCONNECT)
		//{
		//	packet >> ix;
		//	// We got kicked
		//	if (ix == -1)
		//	{
		//		this->disconnectClient();
		//	}
		//	// Other client got disconnected
		//	else
		//	{
		//		for (int i = 0; i < otherPlayersServerId.size(); i++)
		//		{
		//			if (otherPlayersServerId[i] == ix)
		//			{
		//				if (otherPlayers.size() < i)
		//				{
		//					return;
		//				}
		//				sceneHandler->getScene()->removeEntity(otherPlayers[i].first);
		//				this->otherPlayers.erase(otherPlayers.begin() + i);
		//				this->otherPlayersServerId.erase(otherPlayersServerId.begin() + i);
		//			}
		//		}
		//	}
		//}
		//if (gameEvent == GameEvents::DISCONNECT)
		//{
		//	packet >> ix;
		//	if (ix == -1)
		//	{  //we got kicked
		//		this->disconnectClient();
		//		//sceneHandler->setScene();
		//	}
		//	else
		//	{
		//		for (int i = 0; i < otherPlayersServerId.size(); i++)
		//		{
		//			if (otherPlayersServerId[i] == ix)
		//			{
		//				if (otherPlayers.size() < i)
		//				{
		//					return;
		//				}
		//				sceneHandler->getScene()->removeEntity(otherPlayers[i].first);
		//				this->otherPlayers.erase(otherPlayers.begin() + i);
		//				this->otherPlayersServerId.erase(otherPlayersServerId.begin() + i);
		//			}
		//		}
		//	}
		//}
		//else if (gameEvent == GameEvents::START)
		//{
		//	client->starting();
		//}
		//else if (gameEvent == GameEvents::PlayerJoined)
		//{
		//	std::string playerName;
		//	packet >> playerName;
		//	packet >> ix;
		//	this->createAPlayer(ix, playerName);
		//}
		//else if (gameEvent == GameEvents::SpawnEnemy)
		//{
		//	//ix = what type of enemy
		//	packet >> ix;
		//	//should we really create a new entity everytime?
		//	iy = sceneHandler->getScene()->createEntity();
		//	std::cout << "spawn enemy" << iy << std::endl;
		//	monsters.push_back(iy);
		//	if (ix == 0)//blob
		//	{
		//		sceneHandler->getScene()->setComponent<MeshComponent>(iy, this->networkHandlerMeshes.find("blob")->second);
		//	}
		//	else if (ix == 1)//range
		//	{
		//		sceneHandler->getScene()->setComponent<MeshComponent>(iy, this->networkHandlerMeshes.find("range")->second);
		//	}
		//	else if (ix == 2)//tank
		//	{
		//		sceneHandler->getScene()->setComponent<MeshComponent>(iy, this->networkHandlerMeshes.find("tank")->second);
		//	}
		//	else
		//	{
		//		sceneHandler->getScene()->setComponent<MeshComponent>(iy, 0);
		//	}
		//	packet >> fx >> fy >> fz;
		//	Transform& transform = sceneHandler->getScene()->getComponent<Transform>(iy);
		//	transform.position = glm::vec3(fx, fy, fz);
		//}
		//else if (gameEvent == GameEvents::SpawnEnemies)
		//{
		//	//ix = what type of enemy, iy how many enemies
		//	packet >> ix >> iy;
		//	for (int i = 0; i < iy; i++)
		//	{
		//		iz = sceneHandler->getScene()->createEntity();
		//		if (ix == 0)
		//		{
		//			sceneHandler->getScene()->setComponent<MeshComponent>(iz, (int)this->resourceManger->addMesh("assets/models/Swarm_model.obj"));
		//		}
		//		else
		//		{
		//			sceneHandler->getScene()->setComponent<MeshComponent>(iz, (int)this->resourceManger->addMesh("assets/models/Amogus/source/1.fbx"));
		//		}
		//		//ix = what type of enemy
		//		packet >> fx >> fy >> fz;
		//		Transform& transform = sceneHandler->getScene()->getComponent<Transform>(iy);
		//		transform.position = glm::vec3(fx, fy, fz);
		//	}
		//}
		//else if (gameEvent == GameEvents::Explosion)
		//{
		//	//don't know how this should be implemented right now
		//}
		//else if (gameEvent == GameEvents::MonsterDied)
		//{
		//	//don't know how this should be implemented right now
		//}
		//else if (gameEvent == GameEvents::PlayerShoot)
		//{
		//	//don't know how this should be implemented right now
		//}
		//else if (gameEvent == GameEvents::GAMEDATA)
		//{
		//	packet >> ix;  //nrOfPlayers;
		//	for (int i = 0; i < ix; i++)
		//	{
		//		packet >> iz;
		//		if (iz != this->ID)
		//		{
		//			this->otherPlayersServerId.push_back(iz);
		//		}
		//	}
		//	packet >> this->seed;  //seed nr;
		//}
		//else if (gameEvent == GameEvents::GetLevelSeed)
		//{
		//	packet >> ix;
		//	this->seed = ix;
		//}
		//else if (gameEvent == GameEvents::PlayerDied)
		//{
		//	//don't know how this should be implemented right now
		//}
		//else if (gameEvent == GameEvents::GetPlayerNames)
		//{
		//	packet >> ix;
		//	for (int i = 0; i < ix; i++)
		//	{
		//		std::string playerName;
		//		packet >> iy >> playerName;
		//		bool alreadyHavePlayer = false;
		//		for (int i = 0; i < otherPlayersServerId.size() && !alreadyHavePlayer; i++)
		//		{
		//			if (iy == otherPlayersServerId[i])
		//			{
		//				alreadyHavePlayer = true;
		//			}
		//		}
		//		if (!alreadyHavePlayer)
		//		{
		//			createAPlayer(iy, playerName);
		//		}
		//	}
		//}
		// Custom event
		if (event >= (int)GameEvents::END)
		{
			this->handleUDPEventClient(packet, event);
		}
	}
	//this->handleTCPPacket(client->getTCPDataFromServer());
	//this->handleUDPPacket(client->getUDPDataFromServer());

	//tcp
	//sf::Packet cTCPP = client->getTCPDataFromServer();
	//int gameEvent;
	//while (!cTCPP.endOfPacket())
	//{
	//	cTCPP >> gameEvent;
	//	if (gameEvent == GameEvents::DISCONNECT)
	//	{
	//		cTCPP >> ix;
	//		if (ix == -1)
	//		{  //we got kicked
	//			this->disconnectClient();
	//			//sceneHandler->setScene();
	//		}
	//		else
	//		{
	//			for (int i = 0; i < otherPlayersServerId.size(); i++)
	//			{
	//				if (otherPlayersServerId[i] == ix)
	//				{
	//					if (otherPlayers.size() < i)
	//					{
	//						return;
	//					}
	//					sceneHandler->getScene()->removeEntity(otherPlayers[i].first);
	//					this->otherPlayers.erase(otherPlayers.begin() + i);
	//					this->otherPlayersServerId.erase(otherPlayersServerId.begin() + i);
	//				}
	//			}
	//		}
	//	}
	//	else if (gameEvent == GameEvents::START)
	//	{
	//		client->starting();
	//	}
	//	else if (gameEvent == GameEvents::PlayerJoined)
	//	{
	//		std::string playerName;
	//		cTCPP >> playerName;
	//		cTCPP >> ix;
	//		this->createAPlayer(ix, playerName);
	//	}
	//	else if (gameEvent == GameEvents::SpawnEnemy)
	//	{
	//		//ix = what type of enemy
	//		cTCPP >> ix;
	//		//should we really create a new entity everytime?
	//		iy = sceneHandler->getScene()->createEntity();
	//		std::cout << "spawn enemy" << iy << std::endl;
	//		monsters.push_back(iy);
	//		if (ix == 0)//blob
	//		{
	//			sceneHandler->getScene()->setComponent<MeshComponent>(iy, this->networkHandlerMeshes.find("blob")->second);
	//		}
	//		else if (ix == 1)//range
	//		{
	//			sceneHandler->getScene()->setComponent<MeshComponent>(iy, this->networkHandlerMeshes.find("range")->second);
	//		}
	//		else if (ix == 2)//tank
	//		{
	//			sceneHandler->getScene()->setComponent<MeshComponent>(iy, this->networkHandlerMeshes.find("tank")->second);
	//		}
	//		else
	//		{
	//			sceneHandler->getScene()->setComponent<MeshComponent>(iy, 0);
	//		}
	//		cTCPP >> fx >> fy >> fz;
	//		Transform& transform = sceneHandler->getScene()->getComponent<Transform>(iy);
	//		transform.position = glm::vec3(fx, fy, fz);
	//	}
	//	else if (gameEvent == GameEvents::SpawnEnemies)
	//	{
	//		//ix = what type of enemy, iy how many enemies
	//		cTCPP >> ix >> iy;
	//		for (int i = 0; i < iy; i++)
	//		{
	//			iz = sceneHandler->getScene()->createEntity();
	//			if (ix == 0)
	//			{
	//				sceneHandler->getScene()->setComponent<MeshComponent>(iz, (int)this->resourceManger->addMesh("assets/models/Swarm_model.obj"));
	//			}
	//			else
	//			{
	//				sceneHandler->getScene()->setComponent<MeshComponent>(iz, (int)this->resourceManger->addMesh("assets/models/Amogus/source/1.fbx"));
	//			}
	//			//ix = what type of enemy
	//			cTCPP >> fx >> fy >> fz;
	//			Transform& transform = sceneHandler->getScene()->getComponent<Transform>(iy);
	//			transform.position = glm::vec3(fx, fy, fz);
	//		}
	//	}
	//	else if (gameEvent == GameEvents::Explosion)
	//	{
	//		//don't know how this should be implemented right now
	//	}
	//	else if (gameEvent == GameEvents::MonsterDied)
	//	{
	//		//don't know how this should be implemented right now
	//	}
	//	else if (gameEvent == GameEvents::PlayerShoot)
	//	{
	//		//don't know how this should be implemented right now
	//	}
	//	else if (gameEvent == GameEvents::GAMEDATA)
	//	{
	//		cTCPP >> ix;  //nrOfPlayers;
	//		for (int i = 0; i < ix; i++)
	//		{
	//			cTCPP >> iz;
	//			if (iz != this->ID)
	//			{
	//				this->otherPlayersServerId.push_back(iz);
	//			}
	//		}
	//		cTCPP >> this->seed;  //seed nr;
	//	}
	//	else if (gameEvent == GameEvents::GetLevelSeed)
	//	{
	//		cTCPP >> ix;
	//		this->seed = ix;
	//	}
	//	else if (gameEvent == GameEvents::PlayerDied)
	//	{
	//		//don't know how this should be implemented right now
	//	}
	//	else if (gameEvent == GameEvents::GetPlayerNames)
	//	{
	//		cTCPP >> ix;
	//		for (int i = 0; i < ix; i++)
	//		{
	//			std::string playerName;
	//			cTCPP >> iy >> playerName;
	//			bool alreadyHavePlayer = false;
	//			for (int i = 0; i < otherPlayersServerId.size() && !alreadyHavePlayer; i++)
	//			{
	//				if (iy == otherPlayersServerId[i])
	//				{
	//					alreadyHavePlayer = true;
	//				}
	//			}
	//			if (!alreadyHavePlayer)
	//			{
	//				createAPlayer(iy, playerName);
	//			}
	//		}
	//	}
	//}

	//sf::Packet cUDPP = client->getUDPDataFromServer();
	//while (!cUDPP.endOfPacket())
	//{
	//	cUDPP >> gameEvent;
	//	if (gameEvent == GameEvents::UpdatePlayerPos)
	//	{
	//		//ix = amount of players
	//		cUDPP >> ix;
	//		for (int i = 0; i < ix; i++)
	//		{
	//			//fxyz position, fabc rotation
	//			cUDPP >> fx >> fy >> fz >> fa >> fb >> fc;
	//			while (otherPlayers.size() < i)
	//			{
	//				otherPlayers.push_back(std::pair<int, std::string>(sceneHandler->getScene()->createEntity(), "unknown"));
	//			}
	//			Transform& transform = sceneHandler->getScene()->getComponent<Transform>(otherPlayers[i].first);
	//			transform.position = glm::vec3(fx, fy, fz);
	//			transform.rotation = glm::vec3(fa, fb, fc);
	//		}
	//	}
	//	else if (gameEvent == GameEvents::UpdateMonsterPos)
	//	{
	//		//ix number of monsters
	//		cUDPP >> ix;
	//		if (monsters.size() < ix)
	//		{
	//			std::cout << "updating size of monster" << std::endl;
	//			monsters.reserve(ix);
	//			for (int i = monsters.size(); i < ix; i++)
	//			{
	//				iy = sceneHandler->getScene()->createEntity();
	//				monsters.push_back(iy);
	//				sceneHandler->getScene()->setComponent<MeshComponent>(iy);
	//			}
	//		}
	//		for (int i = 0; i < ix; i++)
	//		{
	//			//fxyz position, fabc rotation
	//			cUDPP >> fx >> fy >> fz >> fa >> fb >> fc;
	//			Transform& transform = sceneHandler->getScene()->getComponent<Transform>(monsters[i]);
	//			transform.position = glm::vec3(fx, fy, fz);
	//			transform.rotation = glm::vec3(fa, fb, fc);
	//		}
	//	}
	//}
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
	if (seed == -1)
	{
		for (int i = 0; i < 5 && seed == -1; i++)
		{
			Sleep(1000);
			update();
		}
	}
	return seed;
}

void NetworkHandler::sendAIPolygons(std::vector<glm::vec2> points)
{
	TCPPacketEvent polygonEvent;

	polygonEvent.event = GameEvents::POLYGON_DATA;  //change this
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

const std::string& NetworkHandler::getClientName()
{
	return this->playerName;
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

void NetworkHandler::disconnectClient()
{
	client->disconnect();
	if (client != nullptr)
	{
		delete client;
		client = nullptr;
	}
}
