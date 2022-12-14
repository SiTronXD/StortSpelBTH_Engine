#include "pch.h"
#include "Client.h"
#include <iostream>

Client::Client(const std::string& clientName)
{
    this->isConnectedToServer           = false;
    this->currentTimeToSendDataToServer = 0;
    this->timeToSendDataToServer        = ServerUpdateRate;
    this->clientName                    = clientName;

    this->cleanPackageAndGameEvents();
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
		return false;
	}
	// Bind udp socket
	if (this->udpSocket.bind(UDP_PORT_CLIENT + tries) != sf::Socket::Done)
	{
		Log::write("Client: error with client udpSocket");
		return connect(serverIP, tries);  // Try again
	}
	// Connect the tcp socket to server
	else if (this->tcpListener.listen(TCP_PORT_CLIENT + tries) == sf::Socket::Done)
	{
		Log::write("Client: connecting to server");
		if (this->tcpSocket.connect(serverIP, TCP_PORT_SERVER) == sf::Socket::Done)
		{
			Log::write("Client: connected to server sending name");

			// Send name to server
			sf::Packet infoPacket;
			infoPacket << clientName << (unsigned short)(UDP_PORT_CLIENT + tries);
			this->tcpSocket.send(infoPacket);
		}
		else
		{
			Log::write("Client: error with client tcpSocket");
			this->isConnectedToServer = false;
			this->tcpSocket.setBlocking(false);
			this->udpSocket.setBlocking(false);
			return false;
		}
	}
	else
	{
		Log::write("Client: error with client tcpSocket");
		return connect(serverIP, tries);  // Try again
	}

	// We have now made a full connection with server
	this->isConnectedToServer = true;
	this->tcpSocket.setBlocking(false);
	this->udpSocket.setBlocking(false);
	return true;
}

bool Client::isConnected() const
{
    return this->isConnectedToServer;
}

float Client::getAccumulatedTime() const
{
	return this->currentTimeToSendDataToServer;
}

void Client::update(const float& dt)
{
    this->currentTimeToSendDataToServer += dt;

    // Time To send data to server
    if (this->currentTimeToSendDataToServer > this->timeToSendDataToServer) {
        this->currentTimeToSendDataToServer = 0;
        sendDataToServer();
        cleanPackageAndGameEvents();
    }
}

void Client::disconnect()
{
    // Send to server that we are going to disconnect
    this->clientTcpPacketSend << (int)NetworkEvent::DISCONNECT;
    if (this->clientTcpPacketSend.getDataSize() > 0) {
        this->tcpSocket.send(clientTcpPacketSend);
    }
    this->tcpSocket.disconnect();
    this->udpSocket.unbind();
    this->isConnectedToServer = false;
}

void Client::sendDataToServer()
{
    if (clientTcpPacketSend.getDataSize() > 0)
    {
        this->tcpSocket.send(clientTcpPacketSend);
    }

    // If we have started we send our position to the server
    if (this->clientUdpPacketSend.getDataSize() > sizeof(uint32_t) && isConnected()) 
    {
        this->udpSocket.send(clientUdpPacketSend, this->serverIP, UDP_PORT_SERVER);
    }
}

void Client::cleanPackageAndGameEvents()
{
    // Clear packages so same data doesn't get sent twice
    this->clientTcpPacketSend.clear();
    this->clientUdpPacketSend.clear();
    this->clientUdpPacketSend << sendPacketID++;
}

sf::Packet Client::getTCPDataFromServer()
{
    sf::Packet tempPacketRecv, returnPacketRecv;
    int timesTryingToRecv = 0;
    while (this->tcpSocket.receive(tempPacketRecv) == sf::Socket::Done && timesTryingToRecv < MAXTIMETRYINGTORECV)
    {
        if (!tempPacketRecv.endOfPacket())
        {
            returnPacketRecv.append(tempPacketRecv.getData(), tempPacketRecv.getDataSize());
        }
        ++timesTryingToRecv;
    }
    return returnPacketRecv;
}

sf::Packet Client::getUDPDataFromServer()
{
    int timesTryingToRecv = 0;
    // Get all the data from udpSocket & handle it to some extent
    sf::Packet     udpPacketRecv, tempPacketRecv;
    sf::IpAddress  sender;
    unsigned short port;
    while (this->udpSocket.receive(tempPacketRecv, sender, port) == sf::Socket::Done && timesTryingToRecv < MAXTIMETRYINGTORECV) 
    {
        ++timesTryingToRecv;
        if (sender == this->serverIP) // If we get something that is not from server
        {
            //check if packet is to old
            uint32_t packetID;
            tempPacketRecv >> packetID;
            if (packetID > this->recvPacketID)
            {
                this->recvPacketID = packetID;
                udpPacketRecv = tempPacketRecv;
            }
        }
    }
    // If we didn't get anything return packet with nothing in it
    return udpPacketRecv;
}
