#pragma once
#include "../../vengine/resource_management/ResourceManager.hpp"
#include "../application/Time.hpp"
#include "../components/MeshComponent.hpp"
#include "Client.h"
#include "ServerEngine/Server.h"

class NetworkHandler
{
  private:
	const float waitTimeForServerCreation = 10.0f;

  private:
	std::thread* serverThread;
	bool shutDownServer;
	bool createdServer;
	Client* client;
	SceneHandler* sceneHandler;
	ResourceManager* resourceManger;
	int ID;
	int player;
	std::string playerName;

	//meshes for networkHandler
	std::map<std::string, int> networkHandlerMeshes;

	//HELPERS
	float fx, fy, fz, fa, fb, fc;
	int ix, iy, iz, ia, ib, ic;

	//data
	int seed;
	std::vector<std::pair<int, std::string>> otherPlayers;
	std::vector<int> otherPlayersServerId;
	std::vector<std::pair<int, int>> monsters;  //first is this id, seconds is servers id
	sf::Packet scenePacket;

	std::vector<int> lua_ints;
	std::vector<float> lua_floats;

	void createAPlayer(int serverId, const std::string& playerName);

	void readTCPPacket(sf::Packet& cTCPP);
	void readUDPPacket(sf::Packet& cUDPP);

  public:
	NetworkHandler();
	virtual ~NetworkHandler();
	void setSceneHandler(SceneHandler* sceneHandler);
	void setResourceManager(ResourceManager* resourceManager);

	void setMeshes(const std::string& meshName, const int meshID);

	//SERVER
	void createServer(NetworkScene* serverGame = nullptr);  //create new server on a new thread
	void deleteServer();

	//CLIENT
	Client* createClient(const std::string& name = "BOB");  //should return a client
	Client* getClient();
	bool connectClientToThis();
	bool connectClient(const std::string& serverIP);
	void disconnectClient();
	const bool hasServer();
	void sendAIPolygons(std::vector<glm::vec2> points);
	const std::string& getClientName();
	sf::Packet& getScenePacket();

	//USER
	void updateNetwork();
	void sendTCPDataToClient(TCPPacketEvent tcpP);
	void setPlayerNetworkHandler(int playerID);
	void getLuaData(std::vector<int>& ints, std::vector<float>& floats);
	const std::vector<std::pair<int, std::string>> getPlayers();
	void createPlayers();

	//little cheating here but only one event from client to server via udp
	void sendUDPDataToClient(const glm::vec3& pos, const glm::vec3& rot);
	int getServerSeed();
};