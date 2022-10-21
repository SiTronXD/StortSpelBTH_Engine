#include "Server.h"
#include <iostream>
#include "../ServerGameModes/DefaultServerGame.h"

//can I do this better?
void ConnectUsers(std::vector<clientInfo*>& client, sf::TcpListener& listener, StartingEnum& start)
{
	int id = 0;
	//say we already have a client but he's not connected
	client.resize(1);
	client[client.size() - 1] = new clientInfo("");

	//while the game has NOT started look for players
	while (!start)
	{
		//if we got a connection
		if (listener.accept(client[client.size() - 1]->clientTcpSocket) == sf::Socket::Done)
		{
			id++;
			//client[client.size() - 1]->port = UDP_PORT_CLIENT;
			//std::cout << UDP_PORT_CLIENT << std::endl;

			client[client.size() - 1]->sender = client[client.size() - 1]->clientTcpSocket.getRemoteAddress();  //may be wrong address here 2?

			client[client.size() - 1]->id = id;

			std::cout << "Server: " << client[client.size() - 1]->clientTcpSocket.getRemoteAddress().toString() << " Connected" << std::endl;

			bool duplicatedUser = false;
			// TODO: double check so we don't get double players
			for (int c = 0; c < client.size() - 1; c++)
			{
				if (client[client.size() - 1]->sender == client[c]->sender &&
				    client[client.size() - 1]->clientTcpSocket.getRemotePort() == client[c]->clientTcpSocket.getRemotePort())
				{

					//delete the client before
					delete client[c];
					client.erase(client.begin() + c);

					std::cout << "DUBBLE" << std::endl;

					duplicatedUser = true;
				}
			}

			//get name of player
			//should do a check if we don't get name set blocking false
			client[client.size() - 1]->clientTcpSocket.setBlocking(true);

			sf::Packet socketData;
			client[client.size() - 1]->clientTcpSocket.receive(socketData);
			socketData >> client[client.size() - 1]->name;
			socketData >> client[client.size() - 1]->port;
			std::cout << "Server: " << client[client.size() - 1]->name << " joined the lobby" << std::endl;

			client[client.size() - 1]->clientTcpSocket.setBlocking(false);

			//TODO: send to player their id
			sf::Packet idPacket;
			idPacket << GameEvents::ID << client[client.size() - 1]->id << (int)client.size() - 1;
			client[client.size() - 1]->clientTcpSocket.send(idPacket);

			if (!duplicatedUser)
			{

				//TODO: send that a player has joined
				sf::Packet playerJoinedPacket;
				playerJoinedPacket << GameEvents::PlayerJoined << client[client.size() - 1]->id;
				for (int i = 0; i < client.size() - 1; i++)
				{
					client[i]->clientTcpSocket.send(playerJoinedPacket);
				}
			}
			//create a new client that is ready
			client.resize(client.size() + 1);
			client[client.size() - 1] = new clientInfo("");
		}
		//if we have a client try to recv "start" and start the game
		if (client.size() > 0)
		{
			sf::Packet packet;
			if (client[0]->clientTcpSocket.receive(packet) == sf::Socket::Done)
			{
				int event;
				if (!packet.endOfPacket())
				{
					packet >> event;
					if (event == GameEvents::START)
					{
						std::cout << "Server: start" << std::endl;
						start = StartingEnum::Start;
					}
				}
			}
		}
	}

	//delete a client that we don't have
	delete client[client.size() - 1];
	client.resize(client.size() - 1);
}

Server::Server(ServerGameMode* serverGame)
{
	this->scene.setScriptHandler(&this->scriptHandler);
	this->scriptHandler.setScene(&this->scene);
	if (serverGame == nullptr)
	{
		this->serverGame = new DefaultServerGame();
	}
	else
	{
		this->serverGame = serverGame;
	}

	this->serverGame->setScene(&this->scene);
	this->serverGame->setScriptHandler(&this->scriptHandler);

	this->serverGame->GivePacketInfo(this->serverToClientPacketTcp);

	this->starting = StartingEnum::WaitingForUsers;
	this->currentTimeToSend = 0;

	//how long time it should take before sending next message
	this->timeToSend = ServerUpdateRate;

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

	this->udpSocket.setBlocking(false);
	this->listener.setBlocking(false);

	std::cout << "Server: waiting for users to connect" << std::endl;
	this->connectThread = new std::thread(ConnectUsers, std::ref(this->clients), std::ref(this->listener), std::ref(this->starting));
}

Server::~Server()
{
	//if we still waiting for users to connect but we want to shut down server
	if (this->connectThread != nullptr)
	{
		this->starting = StartingEnum::Start;
		this->connectThread->join();
		delete this->connectThread;
	}

	for (int i = 0; i < clients.size(); i++)
	{
		delete this->clients[i];
	}
	delete serverGame;
	scriptHandler.cleanup();
}

void Server::start()
{
	//wait for the thread to be done
	this->connectThread->join();
	delete this->connectThread;
	this->connectThread = nullptr;

	//make packets ready
	this->clientToServerPacketTcp.resize(this->clients.size());
	this->serverToClientPacketTcp.resize(this->clients.size());
	this->clientToServerPacketUdp.resize(this->clients.size());
	this->serverToClientPacketUdp.resize(this->clients.size());

	//send to clients that we shall start
	sf::Packet startPacket;
	//last one is seed
	startPacket << GameEvents::START << GameEvents::GAMEDATA << (int)clients.size();
	for (int i = 0; i < clients.size(); i++)
	{
		startPacket << clients[i]->id;
	}
	startPacket << this->serverGame->getSeed();

	for (int i = 0; i < this->clients.size(); i++)
	{
		this->clients[i]->clientTcpSocket.send(startPacket);
		this->scene.createPlayer();
	}

	this->udpSocket.setBlocking(false);
	this->listener.setBlocking(false);

	std::cout << "Server: printing all users" << std::endl;
	printAllUsers();
	this->starting = StartingEnum::Running;
	this->serverGame->init();
}

bool Server::update(float dt)
{
	//if all users is connected and client host sayd it ok to start
	if (this->starting == StartingEnum::Start)
	{
		this->start();
	}
	else if (this->starting == StartingEnum::Running)
	{
		this->currentTimeToSend += dt;

		for (int i = 0; i < this->clients.size(); i++)
		{
			this->clients[i]->TimeToDisconnect += dt;
		}
		if (clients.size() == 0)
		{
			return true;
		}

		getDataFromUsers();
		if (this->currentTimeToSend > this->timeToSend)
		{

			this->scene.update(this->currentTimeToSend);
			this->scriptHandler.update();
			this->serverGame->update(this->currentTimeToSend);

			this->seeIfUsersExist();
			this->sendDataToAllUsers();
			this->cleanSendPackages();
			this->currentTimeToSend = 0;
		}
		cleanRecvPackages();
	}
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
	for (int i = 0; i < clients.size(); i++)
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
		for (int i = 0; i < clients.size(); i++)
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
					this->serverGame->addPolygon(points);
					points.clear();
					break;
				case GameEvents::REMOVE_POLYGON_DATA:
					std::cout << "We shall start over with polygon data" << std::endl;
					this->serverGame->removeAllPolygons();
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
					Transform& T = this->scene.getComponent<Transform>(this->scene.getPlayer(ClientID));
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
			Transform& T = this->scene.getComponent<Transform>(this->scene.getPlayer(i));
			packet << T.position.x << T.position.y
			       << T.position.z << T.rotation.x
			       << T.rotation.y << T.rotation.z;
		}
	}
	//get all monster position
	packet << GameEvents::UpdateMonsterPos << this->scene.getEnemySize();
	for (int i = 0; i < this->scene.getEnemySize(); i++)
	{
		Transform& T = this->scene.getComponent<Transform>(this->scene.getEnemies(i));
		packet << T.position.x << T.position.y
		       << T.position.z << T.rotation.x
		       << T.rotation.y << T.rotation.z;
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

int Server::getClientSize()
{
	return (int)clients.size();
}
