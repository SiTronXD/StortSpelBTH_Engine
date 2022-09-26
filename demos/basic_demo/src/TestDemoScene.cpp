#include <iostream>
#include "TestDemoScene.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "vengine.h"




TestDemoScene::TestDemoScene()
	: testEntity(-1), testEntity2(-1), boss(-1)
{
	setRoomVar();
}

TestDemoScene::~TestDemoScene()
{
}

void TestDemoScene::init()
{
	std::cout << "Test scene init" << std::endl;

	// Camera
	int camEntity = this->createEntity();
	this->setComponent<Camera>(camEntity, 1.0f);
	this->setMainCamera(camEntity);

	//room setup
	//this->setUpRooms();
}

void TestDemoScene::update()
{

	if (this->entityValid(this->getMainCameraID()))
	{
		glm::vec3 moveVec = glm::vec3(Input::isKeyDown(Keys::A) - Input::isKeyDown(Keys::D), 0.0f, Input::isKeyDown(Keys::W) - Input::isKeyDown(Keys::S));
		Transform& camTransform = this->getComponent<Transform>(this->getMainCameraID());
		camTransform.position += moveVec * 25.0f * Time::getDT();
	}
	
	//Iterate rooms
	//runRoomIteration();
}

void TestDemoScene::setRoomVar()
{
	foundBoss = false;
	bossHealth = 100;
	roomID = 0;
}

void TestDemoScene::setUpRooms()
{
	
	boss = this->createEntity();
	this->setComponent<MeshComponent>(boss);
	this->getComponent<Transform>(boss).position = glm::vec3(-1000.0f, -1000.0f, -1000.0f);
	for (int i = 0; i < 4; i++)
	{
		doors[i] = this->createEntity();
		this->setComponent<MeshComponent>(doors[i]);
	}
	initRooms(*this, rooms, doors, roomID);
	std::cout << "Num rooms: " << rooms.size() << std::endl;
	std::cout << "Slow: WASD" << std::endl << "Fast: HBNM" << std::endl;
}

void TestDemoScene::runRoomIteration()
{
	if (traverseRooms(*this, rooms, doors, roomID, boss, bossHealth, foundBoss, Time::getDT())) {
		std::cout << "You found the exit!\n";
		Transform& bossTransform = this->getComponent<Transform>((boss));
		bossTransform.position = this->getComponent<Transform>(rooms[roomID]).position + glm::vec3(0.0f, 10.0f, 20.0f);
		bossTransform.scale = glm::vec3(10.0f, 5.0f, 5.0f);
		bossTransform.rotation = glm::vec3(bossTransform.rotation.x + (Time::getDT() * 50), bossTransform.rotation.y + (Time::getDT() * 50), bossTransform.rotation.z + (Time::getDT() * 50));
	}
}
