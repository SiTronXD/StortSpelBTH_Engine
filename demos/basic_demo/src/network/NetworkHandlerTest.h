#pragma once

#include <vengine.h>

enum class GameEvent
{
	EMPTY = (int)GameEvents::END + 1,
	SAY_HELLO,
	SAY_BYE
};

class NetworkHandlerTest : public NetworkHandler
{
private:
	enum class Status { WAITING, PLAYERJOINED, START, RUNNING, END };
	Status status;
public:
	virtual void handleTCPEvent(sf::Packet& tcpPacket, int event) override;
	virtual void handleUDPEvent(sf::Packet& udpPacket, int event) override;
};

