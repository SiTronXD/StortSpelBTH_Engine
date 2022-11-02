#pragma once
#include "ServerEngine/Server.h"
#include "Client.h"
#include "../application/Time.hpp"
#include "../components/MeshComponent.hpp"

class NetworkHandler {
  private:

    std::thread*  serverThread;
    bool          shutDownServer;
    Client*       client;
    SceneHandler* sceneHandler;
    int           ID;
    int           player;

    //HELPERS
    float fx, fy, fz, fa, fb, fc;
    int   ix, iy, iz, ia, ib, ic;

    //data
    int              seed;
    std::vector<int> otherPlayers;
    std::vector<int> otherPlayersServerId;
    std::vector<int> monsters;

	std::vector<int> lua_ints;
	std::vector<float> lua_floats;

  public:
    NetworkHandler();
    virtual ~NetworkHandler();
    void setSceneHandler(SceneHandler* sceneHandler);

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
    void getPlayer(int playerID);
	void getLuaData(std::vector<int>& ints, std::vector<float>& floats);
    //little cheating here but only one event from client to server via udp
    void sendUDPDataToClient(const glm::vec3 &pos, const glm::vec3 &rot);
    int  getServerSeed();
};