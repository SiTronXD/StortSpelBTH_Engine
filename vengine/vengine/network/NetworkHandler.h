#pragma once
#include "ServerEngine/Server.h"
#include "Client.h"
#include "../application/Time.hpp"
#include "../components/MeshComponent.hpp"
#include "../../vengine/resource_management/ResourceManager.hpp"

class NetworkHandler {
  private:
	const float waitTimeForServerCreation = 10.0f;

  private:

    std::thread*        serverThread;
    bool                shutDownServer;
	bool                createdServer;
    Client*             client;
    SceneHandler*       sceneHandler;
	ResourceManager*    resourceManger;
    int                 ID;
    int                 player;

    //HELPERS
    float fx, fy, fz, fa, fb, fc;
    int   ix, iy, iz, ia, ib, ic;

    //data
    int                                         seed;
    std::vector<std::pair<int, std::string>>    otherPlayers;
    std::vector<int>                            otherPlayersServerId;
    std::vector<int>                            monsters;

	std::vector<int> lua_ints;
	std::vector<float> lua_floats;

    void createAPlayer(int serverId, const std::string& playerName);

  public:
    NetworkHandler();
    virtual ~NetworkHandler();
    void setSceneHandler(SceneHandler* sceneHandler);
	void setResourceManager(ResourceManager* resourceManager);

    //SERVER
    void createServer(NetworkScene* serverGame = nullptr); //create new server on a new thread
    void deleteServer();

    //CLIENT
    Client* createClient(const std::string &name = "BOB"); //should return a client
    Client* getClient();
    bool    connectClientToThis();
    bool    connectClient(const std::string &serverIP);
    void    disconnectClient();
    const bool hasServer();
	void sendAIPolygons(std::vector<glm::vec2> points);

    //USER
    void updateNetwork();
    void sendTCPDataToClient(TCPPacketEvent tcpP);
    void setPlayerNetworkHandler(int playerID);
	void getLuaData(std::vector<int>& ints, std::vector<float>& floats);
	const std::vector<std::pair<int, std::string>> getPlayers();

    //little cheating here but only one event from client to server via udp
    void sendUDPDataToClient(const glm::vec3 &pos, const glm::vec3 &rot);
    int  getServerSeed();
};