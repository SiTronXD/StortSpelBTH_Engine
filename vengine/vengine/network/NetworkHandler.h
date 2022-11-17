#pragma once
#include "ServerEngine/Server.h"
#include "Client.h"
#include "../application/Time.hpp"
#include "../components/MeshComponent.hpp"
#include "../../vengine/resource_management/ResourceManager.hpp"

class NetworkHandler 
{
private:
	const float waitTimeForServerCreation = 10.0f;

    std::thread*        serverThread;
    bool                shutDownServer;
	bool                createdServer;
    Client*             client;
    ServerStatus        serverStatus;

    SceneHandler*       sceneHandler;
	ResourceManager*    resourceManger;
    int                 ID;
    int                 player;
	std::string         playerName;
    
    //meshes for networkHandler
	std::map<std::string, int> networkHandlerMeshes;

    //data
    std::vector<std::pair<int, std::string>>    otherPlayers;
    std::vector<int>                            otherPlayersServerId;

    void createAPlayer(int serverId, const std::string& playerName);
public:
    NetworkHandler();
    virtual ~NetworkHandler();
    void setSceneHandler(SceneHandler* sceneHandler);
	void setResourceManager(ResourceManager* resourceManager);

    void setMeshes(const std::string& meshName, const int meshID);
    ServerStatus& getStatus() { return this->serverStatus; }

    // Virtual functions (customization)
    virtual void handleTCPEventClient(sf::Packet& tcpPacket, int event);
    virtual void handleUDPEventClient(sf::Packet& udpPacket, int event);
    virtual void handleTCPEventServer(Server* server, int clientID, sf::Packet& tcpPacket, int event);
    virtual void handleUDPEventServer(Server* server, int clientID, sf::Packet& udpPacket, int event);

    // SERVER
    void createServer(NetworkScene* serverGame = nullptr); // Create new server on a new thread
    void deleteServer();

    // CLIENT
    Client* createClient(const std::string &name = "BOB"); // Should return a client
    Client* getClient();
    bool connectClientToThis();
    bool connectClient(const std::string &serverIP);
    void disconnectClient();
    const bool hasServer();
	const std::string &getClientName();

    // USER
    void update();
    void sendDataToServerTCP(sf::Packet packet);
    void sendDataToServerUDP(sf::Packet packet);
    void setPlayerNetworkHandler(int playerID);
	const std::vector<std::pair<int, std::string>> getPlayers();
	void createPlayers();
};