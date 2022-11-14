#pragma once
#include "SFML/Network.hpp"
#include "NetworkEnumAndDefines.h"
#include "../application/SceneHandler.hpp"
#include "glm/vec3.hpp"

class Client 
{
private:
    //data
    std::string clientName;
    std::string serverIP;
    bool        isConnectedToServer;

    StartingEnum startingEnum;
    float        currentTimeToSendDataToServer;
    float        timeToSendDataToServer;

    // Packets
    sf::Packet clientTcpPacketSend;
    sf::Packet clientUdpPacketSend;

    // Sockets
    sf::TcpSocket   tcpSocket;
    sf::TcpListener tcpListener;
    sf::UdpSocket   udpSocket;

    // Functions
    void sendDataToServer();
    void cleanPackageAndGameEvents();

public:
    Client(const std::string& clientName = "BOB");
    virtual ~Client();
    bool connect(const std::string& serverIP = SERVER_IP, int tries = -1);
    void update(const float& dt);

    bool isConnected() const;
    bool hasStarted() const;

    void starting();
    void disconnect();

    // Send events
    void sendTCPEvent(TCPPacketEvent& eventTCP);
    // Only exist one udp event from client to server
    void sendUDPEvent(const GameEvents& gameEvent, const glm::vec3& pos, const glm::vec3& rot);

    sf::Packet& getTcpPacket();

    sf::Packet getTCPDataFromServer();
    sf::Packet getUDPDataFromServer();
};