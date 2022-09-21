#include "Client.h"
#include <iostream>

Client::Client(std::string name)
{
	this->m_starting = StartingEnum::Start;
	this->m_connected = false;
	this->m_currentTimeToSendDataToServer = 0;
	this->m_timeToSendDataToServer = ServerUpdateRate;
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
		this->m_connected = false;
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
			this->m_connected = false;
			getchar();
			return false;
		}
	}
	else {
		std::cout << "Client: error with client tcpSocket" << std::endl;
		this->m_connected = false;
		getchar();
		return false;
	}

	//we have now made a full connection with server
	this->m_connected = true;
	this->m_tcpSocket.setBlocking(false);
	this->m_udpSocket.setBlocking(false);
	return true;
}

bool Client::isConnected()
{
	return this->m_connected;
}

void Client::update(float dt)
{
	this->m_currentTimeToSendDataToServer += dt;

	//Time To send data to server
	if (this->m_currentTimeToSendDataToServer > this->m_timeToSendDataToServer) {
		this->m_currentTimeToSendDataToServer = 0;
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
	this->m_tcpPacketSend << GameEvents::DISCONNECT;
	this->m_tcpPacketSend << GameEvents::END;
	if (this->m_tcpPacketSend.getDataSize() > 0) {
		this->m_tcpSocket.send(m_tcpPacketSend);
	}
	this->m_tcpSocket.disconnect();
	this->m_udpSocket.unbind();
}


void Client::sendTCPEvent(TCPPacketEvent eventTCP)
{
	
	this->m_tcpPacketSend << eventTCP.gameEvent;
	for (int i = 0; i < eventTCP.nrOfInts; i++) {
		this->m_tcpPacketSend << eventTCP.ints[i];
	}
	for (int i = 0; i < eventTCP.floats.size(); i++) {
		this->m_tcpPacketSend << eventTCP.ints[i];
	}
}

void Client::sendUDPEvent(GameEvents gameEvent, glm::vec3 pos, glm::vec3 rot)
{
	//write over the current udp packet
	sf::Packet p;
	p << gameEvent << pos.x << pos.y << pos.z << rot.x << rot.y << rot.z;
	this->m_udpPacketSend = p;
}

sf::Packet& Client::getTcpPacket()
{
	return this->m_tcpPacketSend;
}

void Client::sendDataToServer()
{
	//add a GameEvents::END in the end so we not never sending something to server
	this->m_tcpPacketSend << GameEvents::END;
	this->m_tcpSocket.send(m_tcpPacketSend);

	//if we have started we send our position to the server
	if (this->m_starting == StartingEnum::Running)
	{
		this->m_udpSocket.send(m_udpPacketSend, this->m_serverIP, UDP_PORT_SERVER);
	}
}

void Client::cleanPackageAndGameEvents()
{
	//clear packages so same data doesn't get sent twice
	this->m_tcpPacketSend.clear();
	this->m_udpPacketSend.clear();
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



