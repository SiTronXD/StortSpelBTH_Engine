#include <iostream>
#include "NetworkTestScene.h"
#include "TheServerGame.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"

NetworkTestScene::NetworkTestScene()
{
}

NetworkTestScene::~NetworkTestScene()
{
}

void NetworkTestScene::init()
{
	std::cout << "Test scene init" << std::endl;

	// Camera
	int camEntity = this->createEntity();
	this->setComponent<Camera>(camEntity);
	this->setMainCamera(camEntity);

	///////////////DEBUG OF DEBUG//////////////
	// stone
	this->stone = this->createEntity();
	// Transform component
	Transform& stransform = this->getComponent<Transform>(this->stone);
	stransform.position = glm::vec3(0.f, 0.f, 80.f);
	stransform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
	stransform.scale = glm::vec3(10.0f, 5.0f, 5.0f);
	/////////////////////////////////////////////

	// Create entity (already has transform)
	this->Player = this->createEntity();

	// Transform component
	Transform& transform = this->getComponent<Transform>(this->Player);
	transform.position = glm::vec3(0.f, 0.f, 30.f);
	transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
	transform.scale = glm::vec3(10.0f, 5.0f, 5.0f);

	// Mesh component
	this->setComponent<MeshComponent>(this->Player);
	MeshComponent& meshComp = this->getComponent<MeshComponent>(this->Player);

}

#include "TheServerGame.h"

void NetworkTestScene::update()
{
	if (this->entityValid(this->getMainCameraID()))
	{
		glm::vec3 moveVec = glm::vec3(Input::isKeyDown(Keys::A) - Input::isKeyDown(Keys::D), 0.0f, Input::isKeyDown(Keys::W) - Input::isKeyDown(Keys::S));
		Transform& transform = this->getComponent<Transform>(this->Player);
		transform.position += moveVec * 25.0f * Time::getDT();
		this->getNetworkHandler()->sendUDPDataToClient(transform.position, transform.rotation);
	}
	if (Input::isKeyPressed(Keys::P)) {
		this->getNetworkHandler()->createServer(new TheServerGame());
		this->getNetworkHandler()->createClient();
		this->getNetworkHandler()->connectClientToThis();
		Transform& transform = this->getComponent<Transform>(this->Player);
		transform.rotation = glm::vec3(69, 69, 69);

	}
	if (Input::isKeyPressed(Keys::H)) {
		this->getNetworkHandler()->sendTCPDataToClient(TCPPacketEvent{ GameEvents::START });
	}
	else if (Input::isKeyPressed(Keys::L)) {
		std::string ip, name;
		std::cin >> name;
		std::cin >> ip;
		this->getNetworkHandler()->createClient(name);
		this->getNetworkHandler()->connectClient(ip);
		Transform& transform = this->getComponent<Transform>(this->Player);
		transform.rotation = glm::vec3(77, 77, 77);
	}
	else if (Input::isKeyPressed(Keys::B)) {
		Transform& transform = this->getComponent<Transform>(this->Player);
		transform.position = glm::vec3(0, 0, 0);
	}
}
