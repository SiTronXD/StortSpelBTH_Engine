#include "NetworkHandlerTest.h"

void NetworkHandlerTest::handleTCPEvent(sf::Packet& tcpPacket, int event)
{
	int h;
	switch ((GameEvent)event)
	{
	case GameEvent::SAY_HELLO:
		tcpPacket >> h;
		Log::write("Hello " + std::to_string(h));
		break;
	case GameEvent::SAY_BYE:
		Log::write("Bye");
		break;
	default:
		break;
	}
}

void NetworkHandlerTest::handleUDPEvent(sf::Packet& udpPacket, int event)
{

}
