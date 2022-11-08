#include <iostream>
#include "LobbyScene.h"
#include "TheServerGame.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "NetworkTestScene.h"
#include "../vengine/vengine/network/ServerGameModes/NetworkLobbyScene.h"

LobbyScene::LobbyScene()
{
}

LobbyScene::~LobbyScene()
{
}

void LobbyScene::init()
{
	// Camera
	int camEntity = this->createEntity();
	this->setComponent<Camera>(camEntity);
	this->setMainCamera(camEntity);
}

void LobbyScene::start()
{
	this->getNetworkHandler()->createClient();
}

void LobbyScene::update()
{
	if (Input::isKeyPressed(Keys::B)) {
		this->getNetworkHandler()->createServer(new NetworkLobbyScene());
		if (this->getNetworkHandler()->connectClientToThis())
		{
			std::cout << "connect" << std::endl;
		}
		else
		{
			std::cout << "no Connect" << std::endl;
		}
		//no visulation that we connected
	}

	if (Input::isKeyPressed(Keys::N)) {
		this->getNetworkHandler()->sendTCPDataToClient(TCPPacketEvent{ GameEvents::START });
	}
	if (this->getNetworkHandler()->getClient()->hasStarted()) {
		std::cout << "client is active" << std::endl;
		this->getSceneHandler()->setScene(new NetworkTestScene());
	}

	if (Input::isKeyPressed(Keys::M)) {
		this->getNetworkHandler()->createClient("Cli");
		std::cout << "ip : ";
		std::string ip;
		std::cin >> ip;
		if (ip == "a") {
			ip = "192.168.1.104";
		}
		this->getNetworkHandler()->connectClient(ip);
	}

	this->getNetworkHandler()->sendUDPDataToClient(
		this->getComponent<Transform>(this->Player).position,
		this->getComponent<Transform>(this->Player).rotation
	);
	this->getComponent<Transform>(this->getMainCameraID()).position = this->getComponent<Transform>(this->Player).position;
	if (Input::isKeyDown(Keys::W)) {
		this->getComponent<Transform>(this->Player).position.z += Time::getDT() * 50;
	}
	else if (Input::isKeyDown(Keys::S)) {
		this->getComponent<Transform>(this->Player).position.z -= Time::getDT() * 50;
	}
}
