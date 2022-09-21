#include "Server.h"
#include <iostream>

//can I do this better?
void ConnectUsers(std::vector<clientInfo*>& client, sf::TcpListener& listener, StartingEnum& start) {

	//say we already have a client but he's not connected
	client.resize(1);
	client[client.size() - 1] = new clientInfo("");

	//while the game has NOT started look for players
	while (!start) {
		//if we got a connection
		if (listener.accept(client[client.size() - 1]->clientTcpSocket) == sf::Socket::Done) {
			client[client.size() - 1]->port = UDP_PORT_CLIENT;
			client[client.size() - 1]->sender = client[client.size() - 1]->clientTcpSocket.getRemoteAddress();//may be wrong address here 2?
			std::cout << "Server: " << client[client.size() - 1]->clientTcpSocket.getRemoteAddress().toString() << std::endl;

			//get name of player
			client[client.size() - 1]->clientTcpSocket.setBlocking(true);
			//should do a check if we don't get name set blocking false
			char data[20];
			std::size_t received;
			client[client.size() - 1]->clientTcpSocket.receive(data, 20, received);
			client[client.size() - 1]->name = { data };
			std::cout << "Server: " << client[client.size() - 1]->name << " joined the lobby" << std::endl;
			client[client.size() - 1]->clientTcpSocket.setBlocking(false);

			//send game data to clients


			//create a new client that is ready
			client.resize(client.size() + 1);
			client[client.size() - 1] = new clientInfo("");
			
		}
		//if we have a client try to recv "start" and start the game
		if (client.size() > 0) {
			sf::Packet packet;
			if (client[0]->clientTcpSocket.receive(packet) == sf::Socket::Done) {
				int event;
				packet >> event;
				if (event == GameEvents::START) {
					std::cout << "Server: start" << std::endl;
					start = StartingEnum::Start;
				}

			}
		}
	}

	//delete a client that we don't have
	delete client[client.size() - 1];
	client.resize(client.size() - 1);
}

Server::Server()
{
	sGame.GivePacketInfo(this->serverToClientPacketTcp);
	starting = StartingEnum::WaitingForUsers;
	currentTimeToSend = 0;

	//how long time it should take before sending next message
	timeToSend = ServerUpdateRate;

	//bind socket
	if (udpSocket.bind(UDP_PORT_SERVER) != sf::Socket::Done) {
		std::cout << "Server: error with server udpSocket" << std::endl;
	}
	if (listener.listen(TCP_PORT_SERVER) != sf::Socket::Done) {
		std::cout << "Server: error with server tcp listener" << std::endl;
	}

	//show servers address
	std::cout << "Server: public address " << sf::IpAddress::getPublicAddress() << std::endl;
	std::cout << "Server: local address " << sf::IpAddress::getLocalAddress() << std::endl;

	udpSocket.setBlocking(false);
	listener.setBlocking(false);


	std::cout << "Server: waiting for users to connect" << std::endl;
	connectThread = new std::thread(ConnectUsers, std::ref(clients), std::ref(listener), std::ref(starting));
}

Server::~Server()
{
	for (int i = 0; i < clients.size(); i++) {
		delete clients[i];
	}
}

void Server::start()
{
	//wait for the thread to be done
	connectThread->join();
	delete connectThread;

	//make packets ready 
	clientToServerPacketTcp.resize(clients.size());
	serverToClientPacketTcp.resize(clients.size());
	clientToServerPacketUdp.resize(clients.size());
	serverToClientPacketUdp.resize(clients.size());

	//send to clients that we shall start
	sf::Packet startPacket;
	//last one is seed
	startPacket << GameEvents::START << GameEvents::GAMEDATA << (int)clients.size() << 1174;
	for (int i = 0; i < clients.size(); i++) {
		clients[i]->clientTcpSocket.send(startPacket);
		sGame.createPlayer();
	}

	udpSocket.setBlocking(false);
	listener.setBlocking(false);

	std::cout << "Server: printing all users" << std::endl;
	printAllUsers();
	this->starting = StartingEnum::Running;
}

void Server::update(float dt)
{
	//if all users is connected and client host sayd it ok to start
	if (this->starting == StartingEnum::Start) {
		this->start();
	}
	else if (this->starting == StartingEnum::Running) {
		this->currentTimeToSend += dt;

		for (int i = 0; i < this->clients.size(); i++) {
			this->clients[i]->TimeToDisconnect += dt;
		}

		getDataFromUsers();
		if (this->currentTimeToSend > this->timeToSend) {
			sGame.update(this->currentTimeToSend);
			seeIfUsersExist();
			sendDataToAllUsers();
			cleanSendPackages();
			this->currentTimeToSend = 0;
		}
		cleanRecvPackages();
	}
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
	for (int i = 0; i < clients.size(); i++) {
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
	for (int i = 0; i < clients.size(); i++) {
		clientToServerPacketTcp[i].clear();
		clientToServerPacketUdp[i].clear();
	}
}

void Server::cleanSendPackages()
{
	for (int i = 0; i < clients.size(); i++) {
		serverToClientPacketTcp[i].clear();
		serverToClientPacketUdp[i].clear();
	}
}

void Server::seeIfUsersExist()
{
	static const float MaxTimeUntilDisconnect = 50.f;

	for (int i = 0; i < clients.size(); i++) {
		if (clients[i]->TimeToDisconnect > MaxTimeUntilDisconnect) {
			handleDisconnects(i);
			for (int c = 0; c < clients.size(); c++) {
				serverToClientPacketTcp[c] << (int)GameEvents::DISCONNECT << i;
			}
		}
	}
}

void Server::sendDataToAllUsers()
{
	for (int i = 0; i < clients.size(); i++) {
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
	for (int i = 0; i < clients.size(); i++) {
		if (clients[i]->clientTcpSocket.receive(clientToServerPacketTcp[i]) == sf::Socket::Done) {
			clients[i]->TimeToDisconnect = 0;
			//need to handle packet direct
			handlePacketFromUser(i, true);
		}
	}

	//do I need to change sender and port and the see if they match?
	sf::IpAddress tempIPAddress;
	unsigned short tempPort;
	sf::Packet tempPacket;
	while (udpSocket.receive(tempPacket, tempIPAddress, tempPort) == sf::Socket::Done) //shall I really have a while loop?
	{
		//check whos it from
		for (int i = 0; i < clients.size(); i++) {
			if (clients[i]->sender == tempIPAddress) {
				clients[i]->TimeToDisconnect = 0;
				clientToServerPacketUdp[i] = tempPacket;
				handlePacketFromUser(i, false);
				break;
			}
		}
	}
}

void Server::handlePacketFromUser(int ClientID, bool tcp)
{
	//tcp
	int gameEvent;
	float packetHelper1;
	int packetHelper2;
	if (tcp) {
		while (!clientToServerPacketTcp[ClientID].endOfPacket()) {

			clientToServerPacketTcp[ClientID] >> gameEvent;
			switch (gameEvent) {
			case GameEvents::EMPTY:
				break;
			case GameEvents::Explosion:
				getToAllExeptIDTCP(ClientID, gameEvent);
				clientToServerPacketTcp[ClientID] >> packetHelper1;//radius
				getToAllExeptIDTCP(ClientID, packetHelper1);
				clientToServerPacketTcp[ClientID] >> packetHelper1;//x
				getToAllExeptIDTCP(ClientID, packetHelper1);
				clientToServerPacketTcp[ClientID] >> packetHelper1;//y
				getToAllExeptIDTCP(ClientID, packetHelper1);
				clientToServerPacketTcp[ClientID] >> packetHelper1;//z
				getToAllExeptIDTCP(ClientID, packetHelper1);
				break;
			case GameEvents::PlayerShoot || GameEvents::SpawnEnemy:
				//create a new package to send to the rest of the clients
				getToAllExeptIDTCP(ClientID, gameEvent);
				clientToServerPacketTcp[ClientID] >> packetHelper2;//type
				getToAllExeptIDTCP(ClientID, packetHelper2);
				clientToServerPacketTcp[ClientID] >> packetHelper1;//x
				getToAllExeptIDTCP(ClientID, packetHelper1);
				clientToServerPacketTcp[ClientID] >> packetHelper1;//y
				getToAllExeptIDTCP(ClientID, packetHelper1);
				clientToServerPacketTcp[ClientID] >> packetHelper1;//z
				getToAllExeptIDTCP(ClientID, packetHelper1);
				break;
			case GameEvents::DISCONNECT:
				std::cout << "Server: user disconnected by own choice" << std::endl;
				handleDisconnects(ClientID);
				break;
			case GameEvents::A_Button_Was_Pressed_On_Client:
				std::cout << "Server: client pressed Button wow (TCP)" << std::endl;
				break;
			}
		}
	}
	else {
		//udp
		while (!clientToServerPacketUdp[ClientID].endOfPacket()) {
			clientToServerPacketUdp[ClientID] >> gameEvent;
			switch (gameEvent) {
			case GameEvents::EMPTY:
				break;
			case GameEvents::A_Button_Was_Pressed_On_Client:
				//std::cout << "Server: client pressed Button wow (UDP)" << std::endl;
				break;
			case GameEvents::UpdatePlayerPos:
				//std::cout << "Server: " << clients[ClientID]->name << " updated player Pos" << std::endl;
				clientToServerPacketUdp[ClientID] >> packetHelper1;
				sGame.getServerPlayers()[ClientID].position.x = packetHelper1;
				clientToServerPacketUdp[ClientID] >> packetHelper1;
				sGame.getServerPlayers()[ClientID].position.y = packetHelper1;
				clientToServerPacketUdp[ClientID] >> packetHelper1;
				sGame.getServerPlayers()[ClientID].position.z = packetHelper1;
				clientToServerPacketUdp[ClientID] >> packetHelper1;
				sGame.getServerPlayers()[ClientID].rotation.x = packetHelper1;
				clientToServerPacketUdp[ClientID] >> packetHelper1;
				sGame.getServerPlayers()[ClientID].rotation.y = packetHelper1;
				clientToServerPacketUdp[ClientID] >> packetHelper1;
				sGame.getServerPlayers()[ClientID].rotation.z = packetHelper1;
			}
		}
	}

}

void Server::createUDPPacketToClient(int clientID, sf::Packet& packet)
{
	packet << GameEvents::UpdatePlayerPos << (int)clients.size() - 1;
	//get all player position
	for (int i = 0; i < clients.size(); i++) {
		if (i != clientID) {
			packet << 
				sGame.getServerPlayers()[i].position.x << sGame.getServerPlayers()[i].position.y << sGame.getServerPlayers()[i].position.z << 
				sGame.getServerPlayers()[i].rotation.x << sGame.getServerPlayers()[i].rotation.y << sGame.getServerPlayers()[i].rotation.z;
		}
	}
	//get all monster position
	packet << GameEvents::UpdateMonsterPos << (int)sGame.getServerEntities().size();
	for (int i = 0; i < sGame.getServerEntities().size(); i++) {
		packet <<
			sGame.getServerEntities()[i].position.x << sGame.getServerEntities()[i].position.y << sGame.getServerEntities()[i].position.z <<
			sGame.getServerEntities()[i].rotation.x << sGame.getServerEntities()[i].rotation.y << sGame.getServerEntities()[i].rotation.z;
	}

}

void Server::printAllUsers()
{
	std::cout << "Server: " << std::endl;
	for (int i = 0; i < clients.size(); i++) {
		std::cout << clients[i]->name << std::endl;
	}
}

int Server::getClientSize()
{
	return (int)clients.size();
}


