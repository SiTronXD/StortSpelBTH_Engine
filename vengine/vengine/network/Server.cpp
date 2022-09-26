#include "Server.h"
#include <iostream>

//can I do this better?
void ConnectUsers(std::vector<clientInfo*>& client, sf::TcpListener& listener, StartingEnum& start)
{
	int id = 0;
	//say we already have a client but he's not connected
	client.resize(1);
	client[client.size() - 1] = new clientInfo("");

	//while the game has NOT started look for players
	while (!start) {
		//if we got a connection
		if (listener.accept(client[client.size() - 1]->clientTcpSocket) == sf::Socket::Done) {
			id++;
			client[client.size() - 1]->port = UDP_PORT_CLIENT;
			client[client.size() - 1]->sender =
				client[client.size() - 1]
				->clientTcpSocket.getRemoteAddress(); //may be wrong address here 2?
			client[client.size() - 1]->id = id;
			std::cout << "Server: "
				<< client[client.size() - 1]->clientTcpSocket.getRemoteAddress().toString()
				<< std::endl;

			bool duplicatedUser = false;
			// TODO: double check so we don't get double players
			for (int c = 0; c < client.size() - 1; c++) {
				if (client[client.size() - 1]->sender == client[c]->sender) {
					delete client[c];
					client.erase(client.begin() + c);
					//get name of player
					client[client.size() - 1]->clientTcpSocket.setBlocking(true);
					//should do a check if we don't get name set blocking false
					char        data[20];
					std::size_t received;
					client[client.size() - 1]->clientTcpSocket.receive(data, 20, received);
					client[client.size() - 1]->name = { data };
					std::cout << "Server: " << client[client.size() - 1]->name
						<< " joined the lobby" << std::endl;
					client[client.size() - 1]->clientTcpSocket.setBlocking(false);

					//TODO: send to player their id
					sf::Packet idPacket;
					idPacket << GameEvents::ID << client[client.size()-1]->id << (int)client.size()-1;
					client[client.size() - 1]->clientTcpSocket.send(idPacket);

					std::cout << "DUBBLE" << std::endl;

					duplicatedUser = true;
				}
			}

			if (!duplicatedUser) {
				//get name of player
				client[client.size() - 1]->clientTcpSocket.setBlocking(true);
				//should do a check if we don't get name set blocking false
				char        data[20];
				std::size_t received;
				client[client.size() - 1]->clientTcpSocket.receive(data, 20, received);
				client[client.size() - 1]->name = { data };
				std::cout << "Server: " << client[client.size() - 1]->name << " joined the lobby"
					<< std::endl;
				client[client.size() - 1]->clientTcpSocket.setBlocking(false);

				//TODO: send to player their id
				sf::Packet idPacket;
				idPacket << GameEvents::ID << client[client.size()-1]->id << (int)client.size();
				client[client.size() - 1]->clientTcpSocket.send(idPacket);

				//TODO: send that a player has joined
				sf::Packet playerJoinedPacket;
				playerJoinedPacket << GameEvents::PlayerJoined << client[client.size()-1]->id;
				for (int i = 0; i < client.size() - 1; i++) {
					client[i]->clientTcpSocket.send(playerJoinedPacket);
				}
			}
			//create a new client that is ready
			client.resize(client.size() + 1);
			client[client.size() - 1] = new clientInfo("");
		}
		//if we have a client try to recv "start" and start the game
		if (client.size() > 0) {
			sf::Packet packet;
			if (client[0]->clientTcpSocket.receive(packet) == sf::Socket::Done) {
				int event;
				if (!packet.endOfPacket()) {
					packet >> event;
					if (event == GameEvents::START) {
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

Server::Server()
{
	this->serverGame.GivePacketInfo(this->serverToClientPacketTcp);
	this->starting = StartingEnum::WaitingForUsers;
	this->currentTimeToSend = 0;

	//how long time it should take before sending next message
	this->timeToSend = ServerUpdateRate;

	//bind socket
	if (this->udpSocket.bind(UDP_PORT_SERVER) != sf::Socket::Done) {
		std::cout << "Server: error with server udpSocket" << std::endl;
	}
	if (this->listener.listen(TCP_PORT_SERVER) != sf::Socket::Done) {
		std::cout << "Server: error with server tcp listener" << std::endl;
	}

	//show servers address
	std::cout << "Server: public address " << sf::IpAddress::getPublicAddress() << std::endl;
	std::cout << "Server: local address " << sf::IpAddress::getLocalAddress() << std::endl;

	this->udpSocket.setBlocking(false);
	this->listener.setBlocking(false);


	std::cout << "Server: waiting for users to connect" << std::endl;
	this->connectThread = new std::thread(ConnectUsers, std::ref(this->clients),
		std::ref(this->listener), std::ref(this->starting));
}

Server::~Server()
{
	//if we still waiting for users to connect but we want to shut down server
	if (this->connectThread != nullptr) {
		this->starting = StartingEnum::Start;
		this->connectThread->join();
		delete this->connectThread;
	}

	for (int i = 0; i < clients.size(); i++) {
		delete this->clients[i];
	}
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
	for (int i = 0; i < clients.size(); i++) {
		startPacket << clients[i]->id;
	}
	startPacket << this->serverGame.getSeed();

	for (int i = 0; i < this->clients.size(); i++) {
		this->clients[i]->clientTcpSocket.send(startPacket);
		this->serverGame.createPlayer();
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
	if (this->starting == StartingEnum::Start) {
		this->start();
	}
	else if (this->starting == StartingEnum::Running) {
		this->currentTimeToSend += dt;

		for (int i = 0; i < this->clients.size(); i++) {
			this->clients[i]->TimeToDisconnect += dt;
		}
		if (clients.size() == 0) {
			return true;
		}
		getDataFromUsers();
		if (this->currentTimeToSend > this->timeToSend) {
			this->serverGame.update(this->currentTimeToSend);
			this->seeIfUsersExist();
			this->sendDataToAllUsers();
			this->cleanSendPackages();
			this->currentTimeToSend = 0;
		}
		cleanRecvPackages();
	}
	return false; //server is not done
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

	sf::Packet packetOtherClients;
	packetOtherClients << GameEvents::DISCONNECT << clients[clientID]->id;
	for(int i = 0; i < clients.size(); i++){
		if(clientID != i){
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
	static const float MaxTimeUntilDisconnect = 10.f;

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
	sf::IpAddress  tempIPAddress;
	unsigned short tempPort;
	sf::Packet     tempPacket;
	while (udpSocket.receive(tempPacket, tempIPAddress, tempPort) ==
		sf::Socket::Done) //shall I really have a while loop?
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

void Server::handlePacketFromUser(const int& ClientID, bool tcp)
{
	//tcp
	int   gameEvent;
	float packetHelper1;
	int   packetHelper2;
	if (tcp) {
		while (!clientToServerPacketTcp[ClientID].endOfPacket()) {
			clientToServerPacketTcp[ClientID] >> gameEvent;
			switch (gameEvent) {
			case GameEvents::EMPTY:
				break;
			case GameEvents::Explosion:
				getToAllExeptIDTCP(ClientID, gameEvent);
				clientToServerPacketTcp[ClientID] >> packetHelper1; //radius
				getToAllExeptIDTCP(ClientID, packetHelper1);
				clientToServerPacketTcp[ClientID] >> packetHelper1; //x
				getToAllExeptIDTCP(ClientID, packetHelper1);
				clientToServerPacketTcp[ClientID] >> packetHelper1; //y
				getToAllExeptIDTCP(ClientID, packetHelper1);
				clientToServerPacketTcp[ClientID] >> packetHelper1; //z
				getToAllExeptIDTCP(ClientID, packetHelper1);
				break;
			case GameEvents::PlayerShoot || GameEvents::SpawnEnemy:
				//create a new package to send to the rest of the clients
				getToAllExeptIDTCP(ClientID, gameEvent);
				clientToServerPacketTcp[ClientID] >> packetHelper2; //type
				getToAllExeptIDTCP(ClientID, packetHelper2);
				clientToServerPacketTcp[ClientID] >> packetHelper1; //x
				getToAllExeptIDTCP(ClientID, packetHelper1);
				clientToServerPacketTcp[ClientID] >> packetHelper1; //y
				getToAllExeptIDTCP(ClientID, packetHelper1);
				clientToServerPacketTcp[ClientID] >> packetHelper1; //z
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
				serverGame.getServerPlayers()[ClientID].position.x = packetHelper1;
				clientToServerPacketUdp[ClientID] >> packetHelper1;
				serverGame.getServerPlayers()[ClientID].position.y = packetHelper1;
				clientToServerPacketUdp[ClientID] >> packetHelper1;
				serverGame.getServerPlayers()[ClientID].position.z = packetHelper1;
				clientToServerPacketUdp[ClientID] >> packetHelper1;
				serverGame.getServerPlayers()[ClientID].rotation.x = packetHelper1;
				clientToServerPacketUdp[ClientID] >> packetHelper1;
				serverGame.getServerPlayers()[ClientID].rotation.y = packetHelper1;
				clientToServerPacketUdp[ClientID] >> packetHelper1;
				serverGame.getServerPlayers()[ClientID].rotation.z = packetHelper1;
			}
		}
	}
}

void Server::createUDPPacketToClient(const int& clientID, sf::Packet& packet)
{
	packet << GameEvents::UpdatePlayerPos << (int)clients.size() - 1;
	//get all player position
	for (int i = 0; i < clients.size(); i++) {
		if (i != clientID) {
			packet << serverGame.getServerPlayers()[i].position.x
				<< serverGame.getServerPlayers()[i].position.y
				<< serverGame.getServerPlayers()[i].position.z
				<< serverGame.getServerPlayers()[i].rotation.x
				<< serverGame.getServerPlayers()[i].rotation.y
				<< serverGame.getServerPlayers()[i].rotation.z;
		}
	}
	//get all monster position
	packet << GameEvents::UpdateMonsterPos << (int)serverGame.getServerEntities().size();
	for (int i = 0; i < serverGame.getServerEntities().size(); i++) {
		packet << serverGame.getServerEntities()[i].position.x
			<< serverGame.getServerEntities()[i].position.y
			<< serverGame.getServerEntities()[i].position.z
			<< serverGame.getServerEntities()[i].rotation.x
			<< serverGame.getServerEntities()[i].rotation.y
			<< serverGame.getServerEntities()[i].rotation.z;
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
