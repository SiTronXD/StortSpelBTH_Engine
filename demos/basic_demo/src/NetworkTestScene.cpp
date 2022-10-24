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
    int camEntity = this->createEntity();
    this->setComponent<Camera>(camEntity, 1.0f);
    this->setMainCamera(camEntity);
    Transform& camTransform = this->getComponent<Transform>(camEntity);
    camTransform.position   = glm::vec3(1.0f);

    this->Player = this->createEntity();

    Transform& transform = this->getComponent<Transform>(this->Player);
    transform.position = glm::vec3(0.f, 0.f, 5.f);
    transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
    transform.scale = glm::vec3(5.0f);

    //ground
    int ground = this->createEntity();
    int groundMesh =
        this->getResourceManager()->addMesh("vengine_assets/models/Cube.fbx");

    this->setComponent<Transform>(ground);
    this->setComponent<MeshComponent>(ground, groundMesh);
    Transform& transform2 = this->getComponent<Transform>(ground);
    transform2.position = glm::vec3(0.0f, -10.0f, 0.0f);
    transform2.scale = glm::vec3(100.f, 0.1f, 100.f);

    int puzzleCreator = this->createEntity();
    this->setScriptComponent(puzzleCreator, "src/Scripts/PuzzleCreatorLua.lua");

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
    if (Input::isKeyPressed(Keys::O)) {
        std::cout << this->getComponent<Transform>(Player).position.z << std::endl;
    }
    this->getNetworkHandler()->sendUDPDataToClient(
        this->getComponent<Transform>(Player).position,
        this->getComponent<Transform>(Player).rotation
    );
}
