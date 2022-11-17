#include "pch.h"
#include "Server.h"
#include <iostream>
#include "../ServerGameModes/DefaultServerGame.h"
#include "../NetworkHandler.h"

bool Server::duplicateUser()
{
	for (int c = 0; c < clients.size() - 1; c++)
	{
		if (tempClient->sender == clients[c]->sender &&
			tempClient->clientTcpSocket.getRemotePort() == clients[c]->clientTcpSocket.getRemotePort())
		{
			// Delete the client before
			delete clients[c];
			clients.erase(clients.begin() + c);

			std::cout << "DUBBLE" << std::endl;
			return true;
		}
	}
	return false;
}

void Server::ConnectUsers()
{
	static int id = 0;

	// If we got a connection
	if (listener.accept(tempClient->clientTcpSocket) == sf::Socket::Done)
	{
		id++;

		tempClient->sender = tempClient->clientTcpSocket.getRemoteAddress();  // May be wrong address here 2?
		tempClient->id = id;

		Log::write("Server: " + tempClient->clientTcpSocket.getRemoteAddress().toString() + " Connected");

		// Double check so we don't get double players
		bool duplicatedUser = false;
		if (clients.size())
		{
			duplicatedUser = duplicateUser();
		}

		// Get name of player
		sf::SocketSelector selector;
		selector.add(tempClient->clientTcpSocket);
		if (!selector.wait(sf::seconds(5.0f)))
		{
			// If we didn't get a name end
			//delete client[client.size() - 1];
			tempClient->clientTcpSocket.disconnect();
			tempClient->clean();
			return;
		}

		sf::Packet socketData;
		tempClient->clientTcpSocket.receive(socketData);
		socketData >> tempClient->name;
		socketData >> tempClient->port;
		Log::write("Server: " + tempClient->name + " joined the lobby");

		clients.push_back(tempClient);
		clientToServerPacketTcp.resize(clients.size());
		serverToClientPacketTcp.resize(clients.size());
		clientToServerPacketUdp.resize(clients.size());
		serverToClientPacketUdp.resize(clients.size());

		if (!duplicatedUser)
		{
			// Send that a player has joined
			sf::Packet playerJoinedPacket;
			playerJoinedPacket << (int)NetworkEvent::CLIENTJOINED << tempClient->name << tempClient->id;
			for (int i = 0; i < clients.size() - 1; i++)
			{
				clients[i]->clientTcpSocket.send(playerJoinedPacket);
			}
		}
		// Create a new client that is ready
		tempClient = new ClientInfo("");
		/*clients[clients.size() - 1]->clientTcpSocket.setBlocking(false);
		clients.resize(clients.size() + 1);
		clients[clients.size() - 1] = new ClientInfo("");*/
	}
}

Server::Server(NetworkHandler* networkHandler, NetworkScene* serverGame)
	: status(ServerStatus::WAITING), networkHandler(networkHandler), tempClient(new ClientInfo(""))
{
	this->tempClient->clientTcpSocket.setBlocking(false);
	this->udpSocket.setBlocking(false);
	this->listener.setBlocking(false);

	// Give info to all 
	this->sceneHandler.setScriptHandler(&this->scriptHandler);
	this->sceneHandler.givePacketInfo(this->serverToClientPacketTcp);
	this->sceneHandler.setPhysicsEngine(&this->physicsEngine);
	this->scriptHandler.setSceneHandler(&this->sceneHandler);
	this->scriptHandler.setPhysicsEngine(&this->physicsEngine);
	this->physicsEngine.setSceneHandler(&this->sceneHandler);

	//this->sceneHandler.setGetClientFunction(std::bind(&Server::startGettingClients, this));
	//this->sceneHandler.setStopClientFunction(std::bind(&Server::stopGettingClients, this));

	// Create scene
	if (serverGame == nullptr)
	{
		sceneHandler.setScene(new DefaultServerGame());
	}
	else
	{
		sceneHandler.setScene(serverGame);
	}
	this->sceneHandler.updateToNextScene();

	// Bind socket
	if (this->udpSocket.bind(UDP_PORT_SERVER) != sf::Socket::Done)
	{
		std::cout << "Server: error with server udpSocket" << std::endl;
	}
	if (this->listener.listen(TCP_PORT_SERVER) != sf::Socket::Done)
	{
		std::cout << "Server: error with server tcp listener" << std::endl;
	}

	// Show servers address
	std::cout << "Server: public address " << sf::IpAddress::getPublicAddress() << std::endl;
	std::cout << "Server: local address " << sf::IpAddress::getLocalAddress() << std::endl;

	this->currentTimeToSend = 0;

	// How long time it should take before sending next message
	this->timeToSend = ServerUpdateRate;
}

Server::~Server()
{
	// If we still waiting for users to connect but we want to shut down server
	this->status = ServerStatus::START;

	for (int i = 0; i < clients.size(); i++)
	{
		delete this->clients[i];
	}
	delete this->tempClient;

	this->scriptHandler.cleanup();
}

void Server::start()
{
	clientToServerPacketTcp.resize(clients.size());
	serverToClientPacketTcp.resize(clients.size());
	clientToServerPacketUdp.resize(clients.size());
	serverToClientPacketUdp.resize(clients.size());

	for (int i = 0; i < this->clients.size(); i++)
	{
		sceneHandler.getScene()->createPlayer();
	}

	this->udpSocket.setBlocking(false);
	this->listener.setBlocking(false);

	std::cout << "Server: printing all users" << std::endl;
	printAllUsers();
	this->status = ServerStatus::RUNNING;
}

bool Server::update(float dt)
{
	// All users are connected and client host says it's ok to start
	if (Input::isKeyPressed(Keys::P))
	{
		this->printAllUsers();
	}
	if (this->status == ServerStatus::START)
	{
		this->start();
	}
	else if (status == ServerStatus::WAITING)
	{
		ConnectUsers();
	}
	else if (status == ServerStatus::RUNNING)
	{
		this->currentTimeToSend += dt;
		for (int i = 0; i < this->clients.size(); i++)
		{
			this->clients[i]->TimeToDisconnect += dt;
		}
		if (this->currentTimeToSend >= this->timeToSend)
		{
			this->sceneHandler.update(this->currentTimeToSend);
			this->scriptHandler.update(this->currentTimeToSend);
			this->physicsEngine.update(this->currentTimeToSend);
			
			this->seeIfUsersExist();
			this->sendDataToAllUsers();
			this->cleanSendPackages();
			
			this->currentTimeToSend -= this->timeToSend;
			if (this->currentTimeToSend > this->timeToSend)
			{
				// Takes to long to load so skip some updates
				this->currentTimeToSend = 0;
			}
			this->sceneHandler.updateToNextScene();
		}
		cleanRecvPackages();
	}
	this->getDataFromUsers();
	
	return false;  // Server is not done
}

std::string Server::getServerIP()
{
	return sf::IpAddress::getPublicAddress().toString();
}

std::string Server::getLocalAddress()
{
	return sf::IpAddress::getLocalAddress().toString();
}

void Server::disconnect()
{
	for (int i = 0; i < clients.size(); i++)
	{
		std::cout << "Server: user " << clients[i]->name << " getting kicked" << std::endl;
		sf::Packet packet;
		packet << (int)NetworkEvent::DISCONNECT << -2;
		clients[i]->clientTcpSocket.send(packet);
		udpSocket.send(packet, clients[i]->sender, clients[i]->port);
		delete clients[i];
	}
	clients.clear();
}

void Server::handleDisconnects(int clientID)
{
	std::cout << "Server: user " << clients[clientID]->name << " getting kicked" << std::endl;

	sf::Packet packetOtherClients;
	packetOtherClients << (int)NetworkEvent::DISCONNECT << clients[clientID]->id;
	for (int i = 0; i < clients.size(); i++)
	{
		if (clientID != i)
		{
			clients[i]->clientTcpSocket.send(packetOtherClients);
		}
	}

	// Say to client we acknowlegde his/her disconnect
	sf::Packet packet;
	packet << (int)NetworkEvent::DISCONNECT << -1;
	clients[clientID]->clientTcpSocket.send(packet);
	udpSocket.send(packet, clients[clientID]->sender, clients[clientID]->port);

	delete clients[clientID];

	this->clients.erase(clients.begin() + clientID);

	this->clientToServerPacketTcp.erase(clientToServerPacketTcp.begin() + clientID);
	this->serverToClientPacketTcp.erase(serverToClientPacketTcp.begin() + clientID);
	this->clientToServerPacketUdp.erase(clientToServerPacketUdp.begin() + clientID);
	this->serverToClientPacketUdp.erase(serverToClientPacketUdp.begin() + clientID);
}

void Server::cleanRecvPackages()
{
	for (int i = 0; i < clients.size(); i++)
	{
		clientToServerPacketTcp[i].clear();
		clientToServerPacketUdp[i].clear();
	}
}

void Server::cleanSendPackages()
{
	for (int i = 0; i < clients.size(); i++)
	{
		serverToClientPacketTcp[i].clear();
		serverToClientPacketUdp[i].clear();
	}
}

void Server::seeIfUsersExist()
{
	static const float MaxTimeUntilDisconnect = 50.f;

	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i]->TimeToDisconnect > MaxTimeUntilDisconnect)
		{
			handleDisconnects(i);
		}
	}
}

void Server::sendDataToAllUsers()
{
	for (int i = 0; i < clients.size(); i++)
	{
		// Send to the client
		if (serverToClientPacketTcp[i].getDataSize())
		{
			clients[i]->clientTcpSocket.send(serverToClientPacketTcp[i]);
		}
	
		// Send UDP
		if (serverToClientPacketUdp[i].getDataSize())
		{
			udpSocket.send(serverToClientPacketUdp[i], clients[i]->sender, clients[i]->port);
		}
	}
}

void Server::getDataFromUsers()
{
	// See if we recv something from
	for (int i = 0; i < clientToServerPacketTcp.size(); i++)
	{
		if (clients[i]->clientTcpSocket.receive(clientToServerPacketTcp[i]) == sf::Socket::Done)
		{
			clients[i]->TimeToDisconnect = 0;
			// Need to handle packet direct
			handlePacketFromUser(i, true);
		}
	}
	
	// Do I need to change sender and port and the see if they match?
	sf::IpAddress tempIPAddress;
	unsigned short tempPort;
	sf::Packet tempPacket;
	while (udpSocket.receive(tempPacket, tempIPAddress, tempPort) == sf::Socket::Done)  // Shall I really have a while loop?
	{
		// Check who it's from
		for (int i = 0; i < clientToServerPacketUdp.size(); i++)
		{
			if (clients[i]->sender == tempIPAddress && clients[i]->port == tempPort)
			{
				clients[i]->TimeToDisconnect = 0;
				clientToServerPacketUdp[i] = tempPacket;
				handlePacketFromUser(i, false);
				break;
			}
		}
	}
}

void Server::handlePacketFromUser(const int& ClientID, bool tcp)
{
	// TCP
	int event;
	float packetHelper1;
	int packetHelper2;
	std::vector<float> points;
	if (tcp)
	{
		while (!clientToServerPacketTcp[ClientID].endOfPacket())
		{
			clientToServerPacketTcp[ClientID] >> event;
			if (event == (int)NetworkEvent::START && this->status == ServerStatus::WAITING && ClientID == 0)
			{
				/*if (this->status == ServerStatus::WAITING)
				{
					delete clients[clients.size() - 1];
					clients.resize(clients.size() - 1);
				}*/
				this->status = ServerStatus::START;
				this->sceneHandler.sendCallFromClient((int)NetworkEvent::START);
			}
			else if (event == GameEvents::GetPlayerNames)
			{
				// Send player names
				sf::Packet playerNamesPacket;

				playerNamesPacket << GameEvents::GetPlayerNames << (int)clients.size() - 2;
				for (int i = 0; i < clients.size() - 2; i++)
				{
					if (ClientID != i)
					{
						playerNamesPacket << clients[i]->id << clients[i]->name;
					}
				}
				clients[ClientID]->clientTcpSocket.send(playerNamesPacket);
			}
			else if (event == (int)NetworkEvent::DISCONNECT)
			{
				std::cout << "Server: user disconnected by own choice" << std::endl;
				handleDisconnects(ClientID);
				return;
			}
			else
			{
				this->networkHandler->handleTCPEventServer(this, ClientID, clientToServerPacketTcp[ClientID], event);
			}
		}
	}

	if (tcp && this->status == ServerStatus::WAITING)
	{
		while (!clientToServerPacketTcp[ClientID].endOfPacket())
		{
			clientToServerPacketTcp[ClientID] >> event;
			if (event == (int)NetworkEvent::START)
			{
				if (ClientID == 0)
				{
					if (this->status == ServerStatus::WAITING)
					{
						delete clients[clients.size() - 1];
						clients.resize(clients.size() - 1);
					}
					this->status = ServerStatus::START;
					this->sceneHandler.sendCallFromClient((int)NetworkEvent::START);
				}
			}
			else if (event == GameEvents::GetPlayerNames)
			{
				// Send player names
				sf::Packet playerNamesPacket;
				
				playerNamesPacket << GameEvents::GetPlayerNames << (int) clients.size() - 2;
				for (int i = 0; i < clients.size() - 2; i++)
				{
					if (ClientID != i)
					{
						playerNamesPacket << clients[i]->id << clients[i]->name;
					}
				}
				clients[ClientID]->clientTcpSocket.send(playerNamesPacket);
			}	
		}
	}
	if (tcp)
	{
		while (!clientToServerPacketTcp[ClientID].endOfPacket())
		{
			clientToServerPacketTcp[ClientID] >> event;
			sf::Packet playerNamesPacket;
			switch (event)
			{
			case GameEvents::EMPTY:
				break;
			case GameEvents::Explosion:
				getToAllExeptIDTCP(ClientID, event);
				clientToServerPacketTcp[ClientID] >> packetHelper1;  //radius
				getToAllExeptIDTCP(ClientID, packetHelper1);
				clientToServerPacketTcp[ClientID] >> packetHelper1;  //x
				getToAllExeptIDTCP(ClientID, packetHelper1);
				clientToServerPacketTcp[ClientID] >> packetHelper1;  //y
				getToAllExeptIDTCP(ClientID, packetHelper1);
				clientToServerPacketTcp[ClientID] >> packetHelper1;  //z
				getToAllExeptIDTCP(ClientID, packetHelper1);
				break;
			case GameEvents::PlayerShoot || GameEvents::SpawnEnemy:
				// Create a new package to send to the rest of the clients
				getToAllExeptIDTCP(ClientID, event);
				clientToServerPacketTcp[ClientID] >> packetHelper2;  //type
				getToAllExeptIDTCP(ClientID, packetHelper2);
				clientToServerPacketTcp[ClientID] >> packetHelper1;  //x
				getToAllExeptIDTCP(ClientID, packetHelper1);
				clientToServerPacketTcp[ClientID] >> packetHelper1;  //y
				getToAllExeptIDTCP(ClientID, packetHelper1);
				clientToServerPacketTcp[ClientID] >> packetHelper1;  //z
				getToAllExeptIDTCP(ClientID, packetHelper1);
				break;
			case (int)NetworkEvent::DISCONNECT:
				std::cout << "Server: user disconnected by own choice" << std::endl;
				handleDisconnects(ClientID);
				return;
			case GameEvents::A_Button_Was_Pressed_On_Client:
				std::cout << "Server: client pressed Button wow (TCP)" << std::endl;
				break;
			case GameEvents::POLYGON_DATA:
				std::cout << "Server: client sent polygon data" << std::endl;
				clientToServerPacketTcp[ClientID] >> packetHelper2;
				points.reserve(packetHelper2 * 2);
				for (int i = 0; i < packetHelper2 * 2; i++)
				{
					clientToServerPacketTcp[ClientID] >> packetHelper1;
					points.push_back(packetHelper1);
				}
				this->sceneHandler.getScene()->addPolygon(points);
				points.clear();
				break;
			case GameEvents::REMOVE_POLYGON_DATA:
				std::cout << "We shall start over with polygon data" << std::endl;
				this->sceneHandler.getScene()->removeAllPolygons();
				break;
				// Calls to Scene
			case (int)NetworkEvent::START:
				std::cout << "a client said start" << std::endl;
				this->sceneHandler.sendCallFromClient((int)NetworkEvent::START);
				break;
			case GameEvents::GetPlayerNames:
				// Send player names
				playerNamesPacket << (int)clients.size();
				for (int i = 0; i < clients.size(); i++)
				{
					playerNamesPacket << clients[i]->id << clients[i]->name;
				}
				clients[ClientID]->clientTcpSocket.send(playerNamesPacket);
				break;
			default: 
				this->networkHandler->handleTCPEventServer(this, ClientID, clientToServerPacketTcp[ClientID], event);
				break;
			}
		}
	}
	else
	{
		// UDP
		while (!clientToServerPacketUdp[ClientID].endOfPacket())
		{
			clientToServerPacketUdp[ClientID] >> event;
			Transform* T;
			switch (event)
			{
			case GameEvents::EMPTY:
				break;
			case GameEvents::A_Button_Was_Pressed_On_Client:
				//std::cout << "Server: client pressed Button wow (UDP)" << std::endl;
				break;
			case GameEvents::UpdatePlayerPos:
				//std::cout << "Server: " << clients[ClientID]->name << " updated player Pos" << std::endl;
				T = &this->sceneHandler.getScene()->getComponent<Transform>(this->sceneHandler.getScene()->getPlayer(ClientID));
				glm::vec3 tempVec;
				clientToServerPacketUdp[ClientID] >> tempVec.x;
				clientToServerPacketUdp[ClientID] >> tempVec.y;
				clientToServerPacketUdp[ClientID] >> tempVec.z;
				T->position = tempVec;
				clientToServerPacketUdp[ClientID] >> tempVec.x;
				clientToServerPacketUdp[ClientID] >> tempVec.y;
				clientToServerPacketUdp[ClientID] >> tempVec.z;
				T->rotation = tempVec;
				break;
			default:
				this->networkHandler->handleUDPEventServer(this, ClientID, clientToServerPacketUdp[ClientID], event);
				break;
			}
		}
	}
}

void Server::createUDPPacketToClient(const int& clientID, sf::Packet& packet)
{
	packet << GameEvents::UpdatePlayerPos << (int)clients.size() - 1;
	//get all player position
	for (int i = 0; i < clients.size(); i++)
	{
		if (i != clientID)
		{
			Transform& T = this->sceneHandler.getScene()->getComponent<Transform>(this->sceneHandler.getScene()->getPlayer(i));
			packet << T.position.x << T.position.y
			       << T.position.z << T.rotation.x
			       << T.rotation.y << T.rotation.z;
		}
	}
	//get all monster position
	packet << GameEvents::UpdateMonsterPos << this->sceneHandler.getScene()->getEnemySize();
	for (int i = 0; i < this->sceneHandler.getScene()->getEnemySize(); i++)
	{
		Transform& T = this->sceneHandler.getScene()->getComponent<Transform>(this->sceneHandler.getScene()->getEnemies(i));
		packet << T.position.x << T.position.y
		       << T.position.z << T.rotation.x
		       << T.rotation.y << T.rotation.z;
	}
}

void Server::startGettingClients() 
{
	clients.resize(clients.size());
	clients[clients.size() - 1] = new ClientInfo("");

	serverToClientPacketUdp.resize(clients.size());
	clientToServerPacketTcp.resize(clients.size());
	clientToServerPacketUdp.resize(clients.size());
	serverToClientPacketTcp.resize(clients.size());
	this->status = ServerStatus::WAITING;
}

void Server::stopGettingClients() 
{
	delete clients[clients.size() - 1];
	clients.resize(clients.size() - 1);
	this->status = ServerStatus::START;
}

void Server::sendToAllClientsTCP(sf::Packet packet)
{
	const void* data = packet.getData();
	size_t size = packet.getDataSize();
	for (int i = 0; i < serverToClientPacketTcp.size(); i++) 
	{
		serverToClientPacketTcp[i].append(data, size);
	}
}

void Server::sendToAllClientsUDP(sf::Packet packet)
{
	const void* data = packet.getData();
	size_t size = packet.getDataSize();
	for (int i = 0; i < serverToClientPacketUdp.size(); i++)
	{
		serverToClientPacketUdp[i].append(data, size);
	}
}

void Server::sendToAllOtherClientsTCP(sf::Packet packet, int clientID)
{
	const void* data = packet.getData();
	size_t size = packet.getDataSize();
	for (int i = 0; i < serverToClientPacketTcp.size(); i++)
	{
		if (i != clientID) { serverToClientPacketTcp[i].append(data, size); }
	}
}

void Server::sendToAllOtherClientsUDP(sf::Packet packet, int clientID)
{
	const void* data = packet.getData();
	size_t size = packet.getDataSize();
	for (int i = 0; i < serverToClientPacketUdp.size(); i++)
	{
		if (i != clientID) { serverToClientPacketUdp[i].append(data, size); }
	}
}

void Server::printAllUsers()
{
	std::cout << "Server: " << std::endl;
	for (int i = 0; i < clients.size(); i++)
	{
		std::cout << clients[i]->name << std::endl;
	}
}