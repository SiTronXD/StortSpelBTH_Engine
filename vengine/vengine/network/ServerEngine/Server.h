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
    uint32_t recvUdpPacketID = 0;

	void clean()
	{
		name = "";
		sender = sf::IpAddress();
		port = 0;
		TimeToDisconnect = 0;
		id = 0;
	}
};

class Server {
private:
	// Functions
	bool duplicateUser();
	// See if user is still connected by seeing if user has been to the sending messages the last 3 seconds or so
	// Else send a message that they have been disconnected
	void seeIfUsersExist();
	void handleDisconnects(int clientID); // If a player have wanted to disconnect
	void cleanRecvPackages();
	void cleanSendPackages();
	void ConnectUsers();

	// To/From users
	void sendDataToAllUsers();
	void handlePacketFromUser(const int& clientID, bool tcp = true);
	void getDataFromUsers();

	// Engine objects
	NetworkSceneHandler sceneHandler;
	PhysicsEngine physicsEngine;
	NetworkScriptHandler scriptHandler;
    ResourceManager resourceManager;

	// Reference used for custom events
	NetworkHandler* networkHandler;
    uint32_t sendUdpPacketID = 0;

	// Print all users
	void printAllUsers();

	// Varibles
	ServerStatus status;
	float        currentTimeToSend;
	float        timeToSend;
	int			 id; // Current ID for new users

	std::vector<ClientInfo*> clients;
	ClientInfo*				 tempClient;
	sf::UdpSocket            udpSocket;
	sf::TcpListener          listener;

	std::vector<sf::Packet> clientToServerPacketTcp; // Packet from client to server i = clientID
	std::vector<sf::Packet> serverToClientPacketTcp;

	std::vector<sf::Packet> clientToServerPacketUdp; // Packet from client to server i = clientID
	std::vector<sf::Packet> serverToClientPacketUdp;

public:
	Server(NetworkHandler* networkHandler, NetworkScene* serverGame = nullptr);
	virtual ~Server();
	void        start();
	bool        update(float dt);
	std::string getServerIP();
	std::string getLocalAddress();
	void        disconnect();

	// Helper functions
	void sendToAllClientsTCP(sf::Packet packet);
	void sendToAllClientsUDP(sf::Packet packet);
	void sendToAllOtherClientsTCP(sf::Packet packet, int clientID);
	void sendToAllOtherClientsUDP(sf::Packet packet, int clientID);
	void sendToClientTCP(sf::Packet packet, int clientID);
	void sendToClientUDP(sf::Packet packet, int clientID);

	template<typename T>
	T* getScene();

	inline int getClientCount() { return (int)this->clients.size(); }
};

template<typename T>
inline T* Server::getScene()
{
	return dynamic_cast<T*>(this->sceneHandler.getScene());
}
