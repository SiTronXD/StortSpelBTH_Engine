#include "pch.h"
#include "NetworkSceneHandler.h"

NetworkSceneHandler::NetworkSceneHandler() {}

void NetworkSceneHandler::update(float dt)
{
	((NetworkScene*)this->getScene())->updateSystems(dt);
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
}

void NetworkSceneHandler::setScene(Scene* scene, std::string path) {
	SceneHandler::setScene(scene, path);
	((NetworkScene*)scene)->givePacketInfo(serverToClientPacketTcp);

	//give new scene players
	if (this->getScene() != nullptr)
	{
		for (int i = 0; i < ((NetworkScene*)this->getScene())->getPlayerSize(); i++)
		{
			((NetworkScene*)scene)->createPlayer();
		}
	}
}

sf::Packet &NetworkSceneHandler::getCallFromClient()
{
	return callsFromClient; 
}

NetworkScene* NetworkSceneHandler::getScene() const
{
	return static_cast<NetworkScene*>(SceneHandler::getScene());
}
