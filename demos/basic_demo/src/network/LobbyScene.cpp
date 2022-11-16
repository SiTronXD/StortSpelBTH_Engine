#include <iostream>
#include "LobbyScene.h"
#include "TheServerGame.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "NetworkTestScene.h"
#include "vengine/network/ServerGameModes/DefaultServerGame.h"
#include "NetworkHandlerTest.h"

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

	this->floor = this->createEntity();
	this->setComponent<MeshComponent>(this->floor);
	Transform& t = this->getComponent<Transform>(this->floor);
	t.position.y = -5.0f;
	t.scale = glm::vec3(25.0f, 1.0f, 25.0f);
}

void LobbyScene::start()
{
	this->getNetworkHandler()->createClient();
}

void LobbyScene::update()
{
	if (Input::isKeyPressed(Keys::B)) {
		this->getNetworkHandler()->createServer(new DefaultServerGame());
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
		this->getNetworkHandler()->sendTCPDataToClient(TCPPacketEvent{ (int)NetworkEvent::START });
	}
	if (Input::isKeyPressed(Keys::D)) {
		this->getNetworkHandler()->disconnectClient();
	}

	if (this->getNetworkHandler()->getStatus() == ServerStatus::RUNNING) {
		std::cout << "Server is active" << std::endl;
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
		else if (ip == "b") {
			ip = "192.168.1.225";
		}
		this->getNetworkHandler()->connectClient(ip);
	}

	if (Input::isKeyPressed(Keys::Q))
	{
		this->getNetworkHandler()->sendTCPDataToClient(TCPPacketEvent{ (int)GameEvent::SAY_HELLO, 1, { 5 } });
	}
	if (Input::isKeyPressed(Keys::E))
	{
		this->getNetworkHandler()->getClient()->getTCPPacket() << (int)GameEvent::SAY_BYE;
	}
	if (Input::isKeyPressed(Keys::U))
	{
		this->getNetworkHandler()->getClient()->getUDPPacket() << (int)GameEvent::SPAM;
	}

	/*this->getNetworkHandler()->sendUDPDataToClient(
		this->getComponent<Transform>(this->Player).position,
		this->getComponent<Transform>(this->Player).rotation
	);*/
	this->getComponent<Transform>(this->getMainCameraID()).position = this->getComponent<Transform>(this->Player).position;
	if (Input::isKeyDown(Keys::W)) {
		this->getComponent<Transform>(this->Player).position.z += Time::getDT() * 50;
	}
	else if (Input::isKeyDown(Keys::S)) {
		this->getComponent<Transform>(this->Player).position.z -= Time::getDT() * 50;
	}
}
