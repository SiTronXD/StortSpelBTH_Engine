#include "Client.h"
#include <iostream>

Client::Client(std::string name)
{
	this->m_starting = StartingEnum::Start;
	this->connected = false;
	this->currentTimeToSendDataToServer = 0;
	this->timeToSendDataToServer = ServerUpdateRate;
	this->name = name;
}

Client::~Client()
{
}

bool Client::connect(std::string serverIP)
{
	this->m_serverIP = serverIP;
	//bind udp socket
	if (this->m_udpSocket.bind(UDP_PORT_CLIENT) != sf::Socket::Done) {
		std::cout << "Client: error with client udpSocket (press something to return)" << std::endl;
		this->connected = false;
		getchar();
		return false;
	}
	//Connect the tcp socket to server
	if (this->m_tcpListener.listen(TCP_PORT_CLIENT) == sf::Socket::Done) {
		std::cout << "Client: connecting to server" << std::endl;
		if (this->m_tcpSocket.connect(m_serverIP, TCP_PORT_SERVER) == sf::Socket::Done) {

			std::cout << "Client: connected to server sending name" << std::endl;

			//send name to server
			this->m_tcpSocket.send(name.c_str(), name.size() + 1);
		}
		else {
			std::cout << "Client: error with client tcpSocket (press something to return)" << std::endl;
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
	this->m_tcpSocket.setBlocking(false);
	this->m_udpSocket.setBlocking(false);
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
	return this->m_starting == StartingEnum::Running;
}

void Client::starting()
{
	this->m_starting = StartingEnum::Running;
}

int Client::getNumberOfPlayers()
{
	//return nrOfPlayers;
	return 0;
}

void Client::disconnect()
{
	//send to server that we are going to disconnect
	this->tcpPacketSend << GameEvents::DISCONNECT;
	this->tcpPacketSend << GameEvents::END;
	if (this->tcpPacketSend.getDataSize() > 0) {
		this->m_tcpSocket.send(tcpPacketSend);
	}
	this->m_tcpSocket.disconnect();
	this->m_udpSocket.unbind();
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
	//write over the current udp packet
	sf::Packet p;
	p << gameEvent << pos.x << pos.y << pos.z << rot.x << rot.y << rot.z;
	this->udpPacketSend = p;
}

sf::Packet& Client::getTcpPacket()
{
	return this->tcpPacketSend;
}

void Client::sendDataToServer()
{
	//add a GameEvents::END in the end so we not never sending something to server
	this->tcpPacketSend << GameEvents::END;
	this->m_tcpSocket.send(tcpPacketSend);

	//if we have started we send our position to the server
	if (this->m_starting == StartingEnum::Running)
	{
		this->m_udpSocket.send(udpPacketSend, this->m_serverIP, UDP_PORT_SERVER);
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
	this->m_tcpSocket.receive(tcpPacketRecv);
	return tcpPacketRecv;
}

sf::Packet Client::getUDPDataFromServer()
{
	//get all the data from udpSocket & handle it to some extent
	sf::Packet udpPacketRecv;
	sf::IpAddress sender;
	unsigned short port;
	if (this->m_udpSocket.receive(udpPacketRecv, sender, port) == sf::Socket::Done) {
		if (sender == this->m_serverIP)//if we get something that is not from server
		{
			return udpPacketRecv;
		}
	}
	//if we didn't get anything return packet with nothing in it
	return sf::Packet();
}



