#include "pch.h"
#include "Client.h"
#include <iostream>

Client::Client(const std::string& clientName)
{
    this->serverStatus                  = ServerStatus::START;
    this->isConnectedToServer           = false;
    this->currentTimeToSendDataToServer = 0;
    this->timeToSendDataToServer        = ServerUpdateRate;
    this->clientName                    = clientName;

    //this->cleanPackageAndGameEvents();
}

Client::~Client() {}

bool Client::connect(const std::string& serverIP, int tries)
{
	this->serverIP = serverIP;
	tries += 4;
	if (tries > 3 * 4)
	{
		this->isConnectedToServer = false;
		this->tcpSocket.setBlocking(false);
		this->udpSocket.setBlocking(false);
		getchar();
		return false;
	}
	//bind udp socket
	if (this->udpSocket.bind(UDP_PORT_CLIENT + tries) != sf::Socket::Done)
	{
		std::cout << "Client: error with client udpSocket (press something to return)" << std::endl;
		return connect(serverIP, tries);  //try again
	}
	//Connect the tcp socket to server
	else if (this->tcpListener.listen(TCP_PORT_CLIENT + tries) == sf::Socket::Done)
	{
		std::cout << "Client: connecting to server" << std::endl;
		if (this->tcpSocket.connect(serverIP, TCP_PORT_SERVER) == sf::Socket::Done)
		{
			std::cout << "Client: connected to server sending name" << std::endl;

			//send name to server
			sf::Packet infoPacket;
			infoPacket << clientName << (unsigned short)(UDP_PORT_CLIENT + tries);
			this->tcpSocket.send(infoPacket);
		}
		else
		{
			std::cout << "Client: error with client tcpSocket (press something to return)" << std::endl;
			this->isConnectedToServer = false;
			this->tcpSocket.setBlocking(false);
			this->udpSocket.setBlocking(false);
			getchar();
			return false;
		}
	}
	else
	{
		std::cout << "Client: error with client tcpSocket" << std::endl;
		return connect(serverIP, tries);  //try again
	}

	//we have now made a full connection with server
	this->isConnectedToServer = true;
	this->tcpSocket.setBlocking(false);
	this->udpSocket.setBlocking(false);
	return true;
}

bool Client::isConnected() const
{
    return this->isConnectedToServer;
}

void Client::update(const float& dt)
{
    this->currentTimeToSendDataToServer += dt;

    //Time To send data to server
    if (this->currentTimeToSendDataToServer > this->timeToSendDataToServer) {
        this->currentTimeToSendDataToServer = 0;
        sendDataToServer();
        cleanPackageAndGameEvents();
    }
}

bool Client::hasStarted() const
{
    return this->serverStatus == ServerStatus::RUNNING;
}

void Client::starting()
{
    this->serverStatus = ServerStatus::RUNNING;
}

void Client::disconnect()
{
    // Send to server that we are going to disconnect
    this->clientTcpPacketSend << (int)GameEvents::DISCONNECT;
    this->clientTcpPacketSend << GameEvents::END;
    if (this->clientTcpPacketSend.getDataSize() > 0) {
        this->tcpSocket.send(clientTcpPacketSend);
    }
    this->tcpSocket.disconnect();
    this->udpSocket.unbind();
}


void Client::sendTCPEvent(TCPPacketEvent& eventTCP)
{
    this->clientTcpPacketSend << eventTCP.event;
    for (int i = 0; i < eventTCP.nrOfInts; i++) {
        this->clientTcpPacketSend << eventTCP.ints[i];
    }
    for (int i = 0; i < eventTCP.floats.size(); i++) {
        this->clientTcpPacketSend << eventTCP.floats[i];
    }
}

void Client::sendUDPEvent(const GameEvents& gameEvent, const glm::vec3& pos, const glm::vec3& rot)
{
    // Write over the current udp packet
    sf::Packet p;
    p << gameEvent << pos.x << pos.y << pos.z << rot.x << rot.y << rot.z;
    this->clientUdpPacketSend = p;
}

sf::Packet& Client::getTcpPacket()
{
    return this->clientTcpPacketSend;
}

void Client::sendDataToServer()
{
    this->tcpSocket.send(clientTcpPacketSend);

    //if we have started we send our position to the server
    if (this->serverStatus == ServerStatus::RUNNING) {
        this->udpSocket.send(clientUdpPacketSend, this->serverIP, UDP_PORT_SERVER);
    }
}

void Client::cleanPackageAndGameEvents()
{
    //clear packages so same data doesn't get sent twice
    this->clientTcpPacketSend.clear();
    this->clientUdpPacketSend.clear();
}

sf::Packet Client::getTCPDataFromServer()
{
    sf::Packet tcpPacketRecv;
    this->tcpSocket.receive(tcpPacketRecv);
    return tcpPacketRecv;
}

sf::Packet Client::getUDPDataFromServer()
{
    //get all the data from udpSocket & handle it to some extent
    sf::Packet     udpPacketRecv;
    sf::IpAddress  sender;
    unsigned short port;
    if (this->udpSocket.receive(udpPacketRecv, sender, port) == sf::Socket::Done) {
        if (sender == this->serverIP) //if we get something that is not from server
        {
            return udpPacketRecv;
        }
    }
    //if we didn't get anything return packet with nothing in it
    return udpPacketRecv;
}
