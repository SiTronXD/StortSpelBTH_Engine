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

    std::thread*        serverThread;
    bool                shutDownServer;
	bool                createdServer;
    Client*             client;
    ServerStatus        serverStatus;

protected:
    SceneHandler*       sceneHandler;
	ResourceManager*    resourceManger;
    int                 ID;
    int                 player;
	std::string         playerName;
    
    // Meshes for networkHandler
	std::map<std::string, int> networkHandlerMeshes;

    // Data
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
    void setStatus(const ServerStatus& status);

    // Virtual functions (customization)
    virtual void handleTCPEventClient(sf::Packet& tcpPacket, int event);
    virtual void handleUDPEventClient(sf::Packet& udpPacket, int event);
    virtual void handleTCPEventServer(Server* server, int clientID, sf::Packet& tcpPacket, int event);
    virtual void handleUDPEventServer(Server* server, int clientID, sf::Packet& udpPacket, int event);
    virtual void onDisconnect(int index);

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
    const bool isConnected();
	const std::string &getClientName();

    // USER
    void update();
    void sendDataToServerTCP(sf::Packet packet);
    void sendDataToServerUDP(sf::Packet packet);
    void setPlayerNetworkHandler(int playerID);
	std::vector<std::pair<int, std::string>> getPlayers();
	void getLuaData(std::vector<int>& ints, std::vector<float>& floats);
	void createPlayers();
	const std::vector<std::pair<int, int>>& getMonsters();

    // HELPER
    glm::vec3 getVec(sf::Packet& packet);
    void sendVec(sf::Packet& packet, const glm::vec3& vec);
};