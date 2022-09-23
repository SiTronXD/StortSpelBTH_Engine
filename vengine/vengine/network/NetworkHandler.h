#pragma once
#include "Server.h"
#include "Client.h"
#include "../application/Time.hpp"
#include "../components/MeshComponent.hpp"

class NetworkHandler {
  private:

    std::thread*  serverThread;
    bool          shutDownServer;
    Client*       client;
    SceneHandler* sceneHandler;

    //HELPERS
    float fx, fy, fz, fa, fb, fc;
    int   ix, iy, iz, ia, ib, ic;

    //data
    int              seed;
    std::vector<int> otherPlayers;
    std::vector<int> monsters;

  public:
    NetworkHandler();
    virtual ~NetworkHandler();
    void setSceneHandler(SceneHandler* sceneHandler);

    //SERVER
    void createServer(); //create new server on a new thread
    void deleteServer();
    //CLIENT
    Client* createClient(const std::string &name = "BOB"); //should return a client
    Client* getClient();
    void    connectClientToThis();
    void    connectClient(const std::string &serverIP);
    void    disconnectClient();

    void updateNetwork();
    void sendTCPDataToClient(TCPPacketEvent &tcpP);
    //little cheating here but only one event from client to server via udp
    void sendUDPDataToClient(const glm::vec3 &pos, const glm::vec3 &rot);
    int  getServerSeed();
};