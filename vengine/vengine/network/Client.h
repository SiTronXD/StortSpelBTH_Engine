#pragma once
#include "SFML/Network.hpp"
#include "NetworkEnumAndDefines.h"
#include "../application/SceneHandler.hpp"
#include "glm/vec3.hpp"

class Client {
public:
	Client(std::string name = "BOB");
	virtual ~Client();
	bool connect(std::string serverIP = SERVER_IP);
	bool isConnected();
	void update(float dt);

	bool hasStarted();
	void starting();
	int getNumberOfPlayers();
	void disconnect();

	//send events
	void sendTCPEvent(TCPPacketEvent eventTCP);
	//only exist one udp event from client to server
	void sendUDPEvent(GameEvents gameEvent, glm::vec3 pos, glm::vec3 rot);
	sf::Packet& getTcpPacket();

	sf::Packet getTCPDataFromServer();
	sf::Packet getUDPDataFromServer();

private:
	sf::TcpSocket m_tcpSocket;
	sf::TcpListener m_tcpListener;
	sf::UdpSocket m_udpSocket;


	std::string m_serverIP;
	bool connected;
	StartingEnum m_starting;
	float currentTimeToSendDataToServer;
	float timeToSendDataToServer;
	sf::Packet tcpPacketSend;
	sf::Packet udpPacketSend;

	void sendDataToServer();
	void cleanPackageAndGameEvents();



	std::string name;
};