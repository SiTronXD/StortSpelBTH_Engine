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

	// Camera
	int camEntity = this->createEntity();
	this->setComponent<Camera>(camEntity);
	this->setMainCamera(camEntity);

	// Create entity (already has transform)
	int testEntity = this->createEntity();

	// Transform component
	Transform& transform = this->getComponent<Transform>(testEntity);
	transform.position = glm::vec3(10.f, 0.f, 30.f);
	//transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
	this->setComponent<MeshComponent>(testEntity, (int)this->getResourceManager()->addMesh("assets/models/fine_ghost.obj"));
	this->setComponent<Collider>(testEntity, Collider::createCapsule(2.0f, 5.0f));
	this->setComponent<Rigidbody>(testEntity);
	this->getComponent<Rigidbody>(testEntity).rotFactor = glm::vec3(0.0f);

	// Create entity (already has transform)
	int puzzleTest = this->createEntity();
	this->setComponent<MeshComponent>(puzzleTest, (int)this->getResourceManager()->addMesh("assets/models/pussel1_5.fbx"));
	this->getComponent<Transform>(puzzleTest).rotation = glm::vec3(0, 180, 0);
	this->getComponent<Transform>(puzzleTest).position = glm::vec3(5, 0, 0);
	addCollisionToScene(this->getResourceManager()->getCollisionShapeFromMesh(this->getResourceManager()->addCollisionShapeFromMesh("assets/models/pussel1_5.fbx")), *this, glm::vec3(5, 0, 0), glm::vec3(0, 180, 0));
	addCollisionToNetworkScene(this->getResourceManager()->getCollisionShapeFromMesh(this->getResourceManager()->addCollisionShapeFromMesh("assets/models/pussel1_5.fbx")), glm::vec3(5, 0, 0), glm::vec3(0, 180, 0));

	// Floor
	int floor = this->createEntity();
	Transform& floorT = this->getComponent<Transform>(floor);
	floorT.position = glm::vec3(10.0f, -25.0f, 30.0f);
	floorT.scale = glm::vec3(100.0f, 1.0f, 100.0f);
	this->setComponent<MeshComponent>(floor, 0);
	this->setComponent<Collider>(floor, Collider::createBox(glm::vec3(100.0f, 1.0f, 100.0f)));

	// Create multiple test rigidbodies
	for (int x = 0; x < 5; x++)
	{
		for (int z = 0; z < 5; z++)
		{
			Entity e = this->createEntity();
			Transform& t = this->getComponent<Transform>(e);
			t.position = glm::vec3(x, 7.5f, z) * 10.0f;
			t.rotation = glm::vec3(rand() % 361, rand() % 361, rand() % 361);
			t.scale = glm::vec3((rand() % 101) * 0.01f + 1.5f);

			this->setComponent<Collider>(e, Collider::createBox(t.scale, rand() % 2));
			this->setComponent<Rigidbody>(e);
			this->setComponent<MeshComponent>(e, 0);
		}
	}


	Entity swarmEntity = this->createEntity();
	this->setComponent<MeshComponent>(swarmEntity);
	Transform& swarmTransform = this->getComponent<Transform>(swarmEntity);
	swarmTransform.position = glm::vec3(10.0f, 0.0f, 30.f);
	MeshComponent& swarmMesh = this->getComponent<MeshComponent>(swarmEntity);
	swarmMesh.meshID = Scene::getResourceManager()->addMesh(
		"assets/models/Swarm_Model.fbx",
		"assets/textures/swarmTextures");

}

#include "TheServerGame.h"

void NetworkTestScene::update()
{
    if (Input::isKeyPressed(Keys::B)) {
        this->getNetworkHandler()->createServer(new TheServerGame());
        this->getNetworkHandler()->createClient();
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
        this->getNetworkHandler()->sendTCPDataToClient(TCPPacketEvent { GameEvents::START });
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
