#pragma once
#include "SFML/Network.hpp"
#include "NetworkEnumAndDefines.h"
#include "SceneHandler.hpp"
#include "glm/vec3.hpp"

class Client {
public:
	Client(std::string name = "BOB");
	virtual ~Client();
	bool connect(std::string serverIP = SERVER_IP);
	bool isConnected();
	void update(float dt);

	bool hasStarted();
	int getNumberOfPlayers();
	void disconnect();

	//send events
	void sendEvent(GameEvents event);
	sf::Packet& getTcpPacket();

	sf::Packet getTCPDataFromServer();
	sf::Packet getUDPDataFromServer();

private:
	sf::TcpSocket tcpSocket;
	sf::TcpListener tcpListener;
	sf::UdpSocket udpSocket;


	std::string serverIP;
	bool connected;
	StartingEnum starting;
	float currentTimeToSendDataToServer;
	float timeToSendDataToServer;
	sf::Packet tcpPacketSend;
	sf::Packet udpPacketSend;

	void sendDataToServer();
	void cleanPackageAndGameEvents();


	std::string name;
};