#pragma once

#include <vengine.h>

enum class GameEvent
{
	EMPTY = (int)GameEvents::END + 1,
	SAY_HELLO,
	SAY_BYE,
	SPAM
};

class NetworkHandlerTest : public NetworkHandler
{
private:
	enum class Status { WAITING, PLAYERJOINED, START, RUNNING, END };
	Status status;
public:
	virtual void handleTCPEventClient(sf::Packet& tcpPacket, int event) override;
	virtual void handleUDPEventClient(sf::Packet& udpPacket, int event) override;
	virtual void handleTCPEventServer(Server* server, int clientID, sf::Packet& tcpPacket, int event) override;
	virtual void handleUDPEventServer(Server* server, int clientID, sf::Packet& udpPacket, int event) override;
};

