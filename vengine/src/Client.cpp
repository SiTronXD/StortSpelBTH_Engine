#include "Client.h"
#include <iostream>

Client::Client(std::string name)
{
	starting = StartingEnum::Start;
	connected = false;
	currentTimeToSendDataToServer = 0;
	timeToSendDataToServer = ServerUpdateRate;
	this->name = name;
}

Client::~Client()
{
}

bool Client::connect(std::string serverIP)
{
	this->serverIP = serverIP;
	//bind udp socket
	if (this->udpSocket.bind(UDP_PORT_CLIENT) != sf::Socket::Done) {
		std::cout << "Client: error with client udpSocket" << std::endl;
		this->connected = false;
		getchar();
		return false;
	}
	//Connect the tcp socket to server
	if (this->tcpListener.listen(TCP_PORT_CLIENT) == sf::Socket::Done) {
		std::cout << "Client: connecting to server" << std::endl;
		if (this->tcpSocket.connect(serverIP, TCP_PORT_SERVER) == sf::Socket::Done) {

			std::cout << "Client: connected to server sending name" << std::endl;

			//send name to server
			this->tcpSocket.send(name.c_str(), name.size() + 1);
		}
		else {
			std::cout << "Client: error with client tcpSocket" << std::endl;
			this->connected = false;
			getchar();
			return false;
		}
	}
	else {
		std::cout << "Client: error with client tcpSocket" << std::endl;
		this->connected = false;
		getchar();
		return false;
	}

	//we have now made a full connection with server
	this->connected = true;
	this->tcpSocket.setBlocking(false);
	this->udpSocket.setBlocking(false);
	return true;
}

bool Client::isConnected()
{
	return this->connected;
}

void Client::update(float dt)
{
	this->currentTimeToSendDataToServer += dt;

	//Time To send data to server
	if (this->currentTimeToSendDataToServer > this->timeToSendDataToServer) {
		this->currentTimeToSendDataToServer = 0;
		sendDataToServer();
		cleanPackageAndGameEvents();
	}
}

bool Client::hasStarted()
{
	return this->starting == StartingEnum::Running;
}

int Client::getNumberOfPlayers()
{
	//return nrOfPlayers;
	return 0;
}

void Client::disconnect()
{
	//send to server that we are going to disconnect
	tcpPacketSend << GameEvents::DISCONNECT;
	tcpPacketSend << GameEvents::END;
	if (tcpPacketSend.getDataSize() > 0) {
		this->tcpSocket.send(tcpPacketSend);
	}
	tcpSocket.disconnect();
	udpSocket.unbind();
}


void Client::sendTCPEvent(TCPPacketEvent eventTCP)
{
	
	this->tcpPacketSend << eventTCP.gameEvent;
	for (int i = 0; i < eventTCP.nrOfInts; i++) {
		this->tcpPacketSend << eventTCP.ints[i];
	}
	for (int i = 0; i < eventTCP.floats.size(); i++) {
		this->tcpPacketSend << eventTCP.ints[i];
	}
}

void Client::sendUDPEvent(GameEvents gameEvent, glm::vec3 pos, glm::vec3 rot)
{
	sf::Packet p;
	p << gameEvent << pos.x << pos.y << pos.z << rot.x << rot.y << rot.z;
	udpPacketSend = p;
}

sf::Packet& Client::getTcpPacket()
{
	return this->tcpPacketSend;
}

void Client::sendDataToServer()
{
	//add a GameEvents::END in the end so we not never sending something to server
	tcpPacketSend << GameEvents::END;
	this->tcpSocket.send(tcpPacketSend);

	//if we have started we send our position to the server
	if (starting == StartingEnum::Running)
	{
		this->udpSocket.send(udpPacketSend, this->serverIP, UDP_PORT_SERVER);
	}
}

void Client::cleanPackageAndGameEvents()
{
	//clear packages so same data doesn't get sent twice
	this->tcpPacketSend.clear();
	this->udpPacketSend.clear();
}

sf::Packet Client::getTCPDataFromServer()
{
	sf::Packet tcpPacketRecv;
	tcpSocket.receive(tcpPacketRecv);
	return tcpPacketRecv;
}

sf::Packet Client::getUDPDataFromServer()
{
	//get all the data from udpSocket & handle it to some extent
	sf::Packet udpPacketRecv;
	sf::IpAddress sender;
	unsigned short port;
	if (udpSocket.receive(udpPacketRecv, sender, port) == sf::Socket::Done) {
		if (sender == this->serverIP)//if we get something that is not from server
		{
			return udpPacketRecv;
		}
	}
	//if we didn't get anything return packet with nothing in it
	return sf::Packet();
}



