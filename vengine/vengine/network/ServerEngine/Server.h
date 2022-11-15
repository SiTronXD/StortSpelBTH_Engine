#pragma once
#include <SFML/Network.hpp>
#include <vector>
#include <thread>
#include "../NetworkEnumAndDefines.h"
#include "NetworkSceneHandler.h"
#include "NetworkScriptHandler.h"

struct ClientInfo {
	ClientInfo(std::string name)
	{
		this->name = name;
	}
	std::string    name;
	sf::IpAddress  sender;
	unsigned short port;
	sf::TcpSocket  clientTcpSocket;
	float          TimeToDisconnect = 0;
	int			   id;
};

class Server {
private:
	// Functions
	// See if user is still connected by seeing if user has been to the sending messages the last 3 seconds or so
	// Else send a message that they have been disconnected
	void seeIfUsersExist();
	void handleDisconnects(int clientID); //if a player have wanted to disconnect
	void cleanRecvPackages();
	void cleanSendPackages();
	void ConnectUsers(std::vector<ClientInfo*>& client, sf::TcpListener& listener, ServerStatus& start);

	// To users
	void sendDataToAllUsers();
	void handlePacketFromUser(const int& clientID, bool tcp = true);

	// From users
	void getDataFromUsers();
	void createUDPPacketToClient(const int& clientID, sf::Packet& packet);

	// Engine objects
	NetworkSceneHandler sceneHandler;
	PhysicsEngine physicsEngine;
	NetworkScriptHandler scriptHandler;

	// Print all users
	void printAllUsers();
	int getClientSize();

	// Varibles
	ServerStatus status;
	float        currentTimeToSend;
	float        timeToSend;

	std::vector<ClientInfo*> clients;
	sf::UdpSocket            udpSocket;
	sf::TcpListener          listener;

	std::vector<sf::Packet> clientToServerPacketTcp; // Packet from client to server i = clientID
	std::vector<sf::Packet> serverToClientPacketTcp;

	std::vector<sf::Packet> clientToServerPacketUdp; // Packet from client to server i = clientID
	std::vector<sf::Packet> serverToClientPacketUdp;

	// Helper functions
	template <typename T> void getToAllExeptIDTCP(int clientID, T var)
	{
		for (int i = 0; i < serverToClientPacketTcp.size(); i++) {
			if (i != clientID) {
				serverToClientPacketTcp[i] << var;
			}
		}
	}
	template <typename T> void getToAllExeptIDUDP(int clientID, T var)
	{
		for (int i = 0; i < serverToClientPacketUdp.size(); i++) {
			if (i != clientID) {
				serverToClientPacketUdp[i] << var;
			}
		}
	}

public:
	Server(NetworkScene* serverGame = nullptr);
	~Server();
	void        start();
	bool        update(float dt);
	std::string getServerIP();
	std::string getLocalAddress();
	void        disconnect();

	void startGettingClients();
	void stopGettingClients();
};