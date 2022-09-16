#pragma once
#include "SFML/Network.hpp"
#include <vector>
#include <thread>
#include "NetworkEnumAndDefines.h"
#include "ServerGame.h"

struct clientInfo {
	clientInfo(std::string name) {
		this->name = name;
	}
	std::string name;
	sf::IpAddress sender;
	unsigned short port;
	sf::TcpSocket clientTcpSocket;
	float TimeToDisconnect = 0;
};

class Server {
public:
	Server();
	~Server();
	void start();
	void update(float dt);
	std::string getServerIP();
	std::string getLocalAddress();
	void disconnect();
private:
	//functions
	//see if user is still connected by seeing if user has been to the sending messages the last 3 seconds or so
	//else send a message that they have been disconnected
	void seeIfUsersExist();
	void handleDisconnects(int clientID); //if a player have wanted to disconnect
	void cleanRecvPackages();
	void cleanSendPackages();

	//sendmessage To users
	void sendDataToAllUsers();
	void getDataFromUsers();
	void handlePacketFromUser(int clientID, bool tcp = true);
	void createUDPPacketToClient(int clientID, sf::Packet& packet);


	//print all users
	void printAllUsers();

	int getClientSize();

	//varibles
	StartingEnum starting;
	float currentTimeToSend;
	float timeToSend;

	//objects
	std::thread* connectThread;

	std::vector<clientInfo*> clients;
	sf::UdpSocket udpSocket;
	sf::TcpListener listener;

	std::vector<sf::Packet> clientToServerPacketTcp; //packet from client to server i = clientID
	std::vector<sf::Packet> serverToClientPacketTcp;
	
	std::vector<sf::Packet> clientToServerPacketUdp; //packet from client to server i = clientID
	std::vector<sf::Packet> serverToClientPacketUdp;


	ServerGame sGame;

	//help functions
	template <typename T>
	void getToAllExeptIDTCP(int clientID, T var) {
		for (int i = 0; i < serverToClientPacketTcp.size(); i++) {
			if (i != clientID) {
				serverToClientPacketTcp[i] << var;
			}
		}
	}
	template <typename T>
	void getToAllExeptIDUDP(int clientID, T var) {
		for (int i = 0; i < serverToClientPacketUdp.size(); i++) {
			if (i != clientID) {
				serverToClientPacketUdp[i] << var;
			}
		}
	}
};