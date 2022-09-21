#pragma once
#include "SFML/Network.hpp"
#include "NetworkEnumAndDefines.h"
#include "../application/SceneHandler.hpp"
#include "glm/vec3.hpp"

class Client {
private:
	//data
	std::string name;
	std::string m_serverIP;
	bool m_connected;

	StartingEnum m_starting;
	float m_currentTimeToSendDataToServer;
	float m_timeToSendDataToServer;

	sf::Packet m_tcpPacketSend;
	sf::Packet m_udpPacketSend;

	//sockets
	sf::TcpSocket m_tcpSocket;
	sf::TcpListener m_tcpListener;
	sf::UdpSocket m_udpSocket;

	//functions
	void sendDataToServer();
	void cleanPackageAndGameEvents();
	
public:
	Client(std::string name = "BOB");
	virtual ~Client();
	bool connect(std::string serverIP = SERVER_IP);
	void update(float dt);

	bool isConnected();
	bool hasStarted();
	int getNumberOfPlayers();//DOESN'T WORK FOR NOW
	
	void starting();
	void disconnect();

	//send events
	void sendTCPEvent(TCPPacketEvent eventTCP);
	//only exist one udp event from client to server
	void sendUDPEvent(GameEvents gameEvent, glm::vec3 pos, glm::vec3 rot);
	sf::Packet& getTcpPacket();

	sf::Packet getTCPDataFromServer();
	sf::Packet getUDPDataFromServer();
};