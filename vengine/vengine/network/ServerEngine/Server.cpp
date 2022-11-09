#include "pch.h"
#include "Server.h"
#include <iostream>
#include "../ServerGameModes/DefaultServerGame.h"

bool duplicateUser(std::vector<clientInfo*>& client) 
{
	for (int c = 0; c < client.size() - 1; c++)
	{
		if (client[client.size() - 1]->sender == client[c]->sender &&
		    client[client.size() - 1]->clientTcpSocket.getRemotePort() == client[c]->clientTcpSocket.getRemotePort())
		{
			//delete the client before
			delete client[c];
			client.erase(client.begin() + c);

			std::cout << "DUBBLE" << std::endl;
			return true;
		}
	}
	return false;
}

    //can I do this better?
void ConnectUsers(std::vector<clientInfo*>& client, sf::TcpListener& listener, StartingEnum& start)
{
	static int id = 0;

	//while the game has NOT started look for players
	if (start == StartingEnum::WaitingForUsers)
	{
		//if we got a connection
		if (listener.accept(client[client.size() - 1]->clientTcpSocket) == sf::Socket::Done)
		{
			id++;

			client[client.size() - 1]->sender = client[client.size() - 1]->clientTcpSocket.getRemoteAddress();  //may be wrong address here 2?
			client[client.size() - 1]->id = id;

			std::cout << "Server: " << client[client.size() - 1]->clientTcpSocket.getRemoteAddress().toString() << " Connected" << std::endl;

			bool duplicatedUser = duplicateUser(client);
			//double check so we don't get double players


			//get name of player
			sf::SocketSelector selector;
			selector.add(client[client.size() - 1]->clientTcpSocket);
			if (!selector.wait(sf::seconds(5.0f)))
			{
				//if we didn't get a name end
				delete client[client.size() - 1];
				client[client.size() - 1] = new clientInfo("");
				return;
			}

			sf::Packet socketData;
			client[client.size() - 1]->clientTcpSocket.receive(socketData);
			socketData >> client[client.size() - 1]->name;
			socketData >> client[client.size() - 1]->port;
			std::cout << "Server: " << client[client.size() - 1]->name << " joined the lobby" << std::endl;

			if (!duplicatedUser)
			{

				// send that a player has joined
				sf::Packet playerJoinedPacket;
				playerJoinedPacket << GameEvents::PlayerJoined << client[client.size() - 1]->name << client[client.size() - 1]->id;
				for (int i = 0; i < client.size(); i++)
				{
					client[i]->clientTcpSocket.send(playerJoinedPacket);
				}
			}
			//create a new client that is ready
			client[client.size() - 1]->clientTcpSocket.setBlocking(false);
			client.resize(client.size() + 1);
			client[client.size() - 1] = new clientInfo("");
		}
	}
}

Server::Server(NetworkScene* serverGame)
{
	this->udpSocket.setBlocking(false);
	this->listener.setBlocking(false);
	//give info to all 
	this->sceneHandler.setScriptHandler(&this->scriptHandler);
	this->sceneHandler.givePacketInfo(this->serverToClientPacketTcp);
	this->sceneHandler.setPhysicsEngine(&this->physicsEngine);
	this->scriptHandler.setSceneHandler(&this->sceneHandler);
	this->scriptHandler.setPhysicsEngine(&this->physicsEngine);
	this->physicsEngine.setSceneHandler(&this->sceneHandler);

	this->sceneHandler.setGetClientFunction(std::bind(&Server::startGettingClients, this));
	this->sceneHandler.setStopClientFunction(std::bind(&Server::stopGettingClients, this));

	//crete scene
	if (serverGame == nullptr)
	{
		sceneHandler.setScene(new DefaultServerGame());
	}
	else
	{
		sceneHandler.setScene(serverGame);
	}
	this->sceneHandler.updateToNextScene();

	//bind socket
	if (this->udpSocket.bind(UDP_PORT_SERVER) != sf::Socket::Done)
	{
		std::cout << "Server: error with server udpSocket" << std::endl;
	}
	if (this->listener.listen(TCP_PORT_SERVER) != sf::Socket::Done)
	{
		std::cout << "Server: error with server tcp listener" << std::endl;
	}

	//show servers address
	std::cout << "Server: public address " << sf::IpAddress::getPublicAddress() << std::endl;
	std::cout << "Server: local address " << sf::IpAddress::getLocalAddress() << std::endl;

	this->currentTimeToSend = 0;

	//how long time it should take before sending next message
	this->timeToSend = ServerUpdateRate;
}

Server::~Server()
{
	//if we still waiting for users to connect but we want to shut down server
	this->starting = StartingEnum::Start;

	for (int i = 0; i < clients.size(); i++)
	{
		delete this->clients[i];
	}

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
	this->starting = StartingEnum::Running;
	
}

bool Server::update(float dt)
{
	//if all users is connected and client host sayd it ok to start
	if (this->starting == StartingEnum::Start)
	{
		this->start();
	}
	else if (starting == StartingEnum::WaitingForUsers)
	{
		ConnectUsers(clients, listener, starting);
	}
	else if (starting == StartingEnum::Running)
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
				//takes to long to load so skip some updates
				this->currentTimeToSend = 0;
			}
			this->sceneHandler.updateToNextScene();
		}
		cleanRecvPackages();
	}
	getDataFromUsers();
	
	return false;  //server is not done
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
		packet << GameEvents::DISCONNECT << -1;
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
	packetOtherClients << GameEvents::DISCONNECT << clients[clientID]->id;
	for (int i = 0; i < clients.size(); i++)
	{
		if (clientID != i)
		{
			clients[i]->clientTcpSocket.send(packetOtherClients);
		}
	}

	//say to client we ack his/her disconnect
	sf::Packet packet;
	packet << GameEvents::DISCONNECT << -1;
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
		//send to the client
		serverToClientPacketTcp[i] << GameEvents::END;
		clients[i]->clientTcpSocket.send(serverToClientPacketTcp[i]);
	
		//send UDP
		sf::Packet sendUDPPacket;
		createUDPPacketToClient(i, sendUDPPacket);
		udpSocket.send(sendUDPPacket, clients[i]->sender, clients[i]->port);
	}
}

void Server::getDataFromUsers()
{
	//see if we recv something from
	if (this->starting == StartingEnum::WaitingForUsers && clients.size() > 0 && 
		clients[0]->clientTcpSocket.receive(clientToServerPacketTcp[0]) == sf::Socket::Done)
	{
		handlePacketFromUser(0, true);
		return;
	}

	for (int i = 0; i < clientToServerPacketTcp.size(); i++)
	{
		if (clients[i]->clientTcpSocket.receive(clientToServerPacketTcp[i]) == sf::Socket::Done)
		{
			clients[i]->TimeToDisconnect = 0;
			//need to handle packet direct
			handlePacketFromUser(i, true);
		}
	}
	
	//do I need to change sender and port and the see if they match?
	sf::IpAddress tempIPAddress;
	unsigned short tempPort;
	sf::Packet tempPacket;
	while (udpSocket.receive(tempPacket, tempIPAddress, tempPort) == sf::Socket::Done)  //shall I really have a while loop?
	{
		//check whos it from
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
	//tcp
	int gameEvent;
	float packetHelper1;
	int packetHelper2;
	std::vector<float> points;
	if (tcp && this->starting == StartingEnum::WaitingForUsers)
	{
		while (!clientToServerPacketTcp[ClientID].endOfPacket())
		{
			clientToServerPacketTcp[ClientID] >> gameEvent;
			if (gameEvent == GameEvents::START)
			{
				if (ClientID == 0)
				{
					if (this->starting == StartingEnum::WaitingForUsers)
					{
						delete clients[clients.size() - 1];
						clients.resize(clients.size() - 1);
					}
					this->starting = StartingEnum::Start;
					this->sceneHandler.sendCallFromClient(GameEvents::START);
				}
			}
		}
	}
	if (tcp)
	{
		while (!clientToServerPacketTcp[ClientID].endOfPacket())
		{
			clientToServerPacketTcp[ClientID] >> gameEvent;
			switch (gameEvent)
			{
				case GameEvents::EMPTY:
					break;
				case GameEvents::Explosion:
					getToAllExeptIDTCP(ClientID, gameEvent);
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
					//create a new package to send to the rest of the clients
					getToAllExeptIDTCP(ClientID, gameEvent);
					clientToServerPacketTcp[ClientID] >> packetHelper2;  //type
					getToAllExeptIDTCP(ClientID, packetHelper2);
					clientToServerPacketTcp[ClientID] >> packetHelper1;  //x
					getToAllExeptIDTCP(ClientID, packetHelper1);
					clientToServerPacketTcp[ClientID] >> packetHelper1;  //y
					getToAllExeptIDTCP(ClientID, packetHelper1);
					clientToServerPacketTcp[ClientID] >> packetHelper1;  //z
					getToAllExeptIDTCP(ClientID, packetHelper1);
					break;
				case GameEvents::DISCONNECT:
					std::cout << "Server: user disconnected by own choice" << std::endl;
					handleDisconnects(ClientID);
					break;
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
					//Calls to Scene
				case GameEvents::START:
					std::cout << "a client said start" << std::endl;
					this->sceneHandler.sendCallFromClient(GameEvents::START);
					break;
			}
		}
	}
	else
	{
		//udp
		while (!clientToServerPacketUdp[ClientID].endOfPacket())
		{
			clientToServerPacketUdp[ClientID] >> gameEvent;
			switch (gameEvent)
			{
				case GameEvents::EMPTY:
					break;
				case GameEvents::A_Button_Was_Pressed_On_Client:
					//std::cout << "Server: client pressed Button wow (UDP)" << std::endl;
					break;
				case GameEvents::UpdatePlayerPos:
				{
					//std::cout << "Server: " << clients[ClientID]->name << " updated player Pos" << std::endl;
					Transform& T = this->sceneHandler.getScene()->getComponent<Transform>(this->sceneHandler.getScene()->getPlayer(ClientID));
					glm::vec3 tempVec;
					clientToServerPacketUdp[ClientID] >> tempVec.x;
					clientToServerPacketUdp[ClientID] >> tempVec.y;
					clientToServerPacketUdp[ClientID] >> tempVec.z;
					T.position = tempVec;
					clientToServerPacketUdp[ClientID] >> tempVec.x;
					clientToServerPacketUdp[ClientID] >> tempVec.y;
					clientToServerPacketUdp[ClientID] >> tempVec.z;
					T.rotation = tempVec;
				}
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
	clients.resize(clients.size() + 1);
	clients[clients.size() - 1] = new clientInfo("");

	serverToClientPacketUdp.resize(clients.size());
	clientToServerPacketTcp.resize(clients.size());
	clientToServerPacketUdp.resize(clients.size());
	serverToClientPacketTcp.resize(clients.size());
	this->starting = StartingEnum::WaitingForUsers;
}

void Server::stopGettingClients() 
{
	delete clients[clients.size() - 1];
	clients.resize(clients.size() - 1);
	this->starting = StartingEnum::Start;
}

void Server::printAllUsers()
{
	std::cout << "Server: " << std::endl;
	for (int i = 0; i < clients.size(); i++)
	{
		std::cout << clients[i]->name << std::endl;
	}
}

int Server::getClientSize()
{
	return (int)clients.size();
}
