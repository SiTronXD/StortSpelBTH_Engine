#include "pch.h"
#include "NetworkLobbyScene.h"

#include "DefaultServerGame.h"

NetworkLobbyScene::NetworkLobbyScene() {}

void NetworkLobbyScene::init() {
	((NetworkSceneHandler*)this->getSceneHandler())->clientGetFunc();
}

void NetworkLobbyScene::update(float dt)
{
	if (((NetworkSceneHandler*)this->getSceneHandler())->getCallFromClient() == GameEvents::START)
	{
		std::cout << "got start in network lobby" << std::endl;
		((NetworkSceneHandler*)this->getSceneHandler())->setScene(new DefaultServerGame());
		this->addEvent({(int)GameEvents::START});
		((NetworkSceneHandler*)this->getSceneHandler())->clientStopFunc();
	}
	
}