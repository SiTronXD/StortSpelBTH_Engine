#include "NetworkSceneHandler.h"

NetworkSceneHandler::NetworkSceneHandler() {}

void NetworkSceneHandler::update(float dt)
{
	((NetworkScene*)this->getScene())->updateSystems(dt);
	//TODO : maybe change script handler to network script handler
	this->getScriptHandler()->updateSystems(((NetworkScene*)this->getScene())->getLuaSystems());
	((NetworkScene*)this->getScene())->update(dt);
}

void NetworkSceneHandler::givePacketInfo(std::vector<sf::Packet>& serverToClient)
{
	this->serverToClientPacketTcp = &serverToClient;
}

void NetworkSceneHandler::updateToNextScene() 
{
	SceneHandler::updateToNextScene();
	((NetworkScene*)this->getScene())->givePacketInfo(*serverToClientPacketTcp);
}

NetworkScene* NetworkSceneHandler::getScene() const
{
	return (NetworkScene*)SceneHandler::getScene();
}
