#include <iostream>
#include "TestDemoScene.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "vengine.h"




TestDemoScene::TestDemoScene()
	: testEntity(-1), testEntity2(-1)
{
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

	/*
	// Create entity (already has transform)
	this->testEntity = this->createEntity();

	// Transform component
	Transform& transform = this->getComponent<Transform>(this->testEntity);
	transform.position = glm::vec3(0.f, 0.f, 30.f);
	transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
	transform.scale = glm::vec3(10.0f, 5.0f, 5.0f);

	// Mesh component
	this->setComponent<MeshComponent>(this->testEntity);
	MeshComponent& meshComp = this->getComponent<MeshComponent>(this->testEntity);

	// Create entity2 (already has transform)
	this->testEntity2 = this->createEntity();

	// Transform component
	Transform& transform2 = this->getComponent<Transform>(this->testEntity2);
	transform2.position = glm::vec3(20.f, 0.f, 30.f);
	transform2.rotation = glm::vec3(-90.0f, 40.0f, 0.0f);
	transform2.scale = glm::vec3(10.0f, 5.0f, 5.0f);

	// Mesh component
	this->setComponent<MeshComponent>(this->testEntity2);
	MeshComponent& meshComp2 = this->getComponent<MeshComponent>(this->testEntity2);
	*/

	//Set up room layout
	initRooms(*this, rooms);
	for (int i = 0; i < 4; i++)
	{
		doors[i] = this->createEntity();
		this->setComponent<MeshComponent>(doors[i]);
	}
	foundBoss = false;
	done = false;
	roomID = 0;
	placeDoors();
	//traverseRoomsConsole(*this, rooms);
}

void TestDemoScene::update()
{
	/*
	Transform& transform2 = this->getComponent<Transform>(this->testEntity2);
	transform2.position.x += Time::getDT();
	*/
	if (this->entityValid(this->getMainCameraID()))
	{
		glm::vec3 moveVec = glm::vec3(Input::isKeyDown(Keys::A) - Input::isKeyDown(Keys::D), 0.0f, Input::isKeyDown(Keys::W) - Input::isKeyDown(Keys::S));
		Transform& camTransform = this->getComponent<Transform>(this->getMainCameraID());
		camTransform.position += moveVec * 25.0f * Time::getDT();
		//std::cout << "(" << camTransform.position.x << ", " << camTransform.position.y << ", " << camTransform.position.z << ")" << std::endl;
	}
	
	if (!done && traverseRooms()) {
		std::cout << "You found the exit!\n";
	}
}

bool TestDemoScene::traverseRooms()
{
	bool ret = false;
	Room curRoom = this->getComponent<Room>(rooms[roomID]);

	if (canGoForward()) {
		roomID = curRoom.up;
		placeDoors();
	}
	else if (canGoBack()) {
		roomID = curRoom.down;
		placeDoors();
	}
	else if (canGoLeft()) {
		roomID = curRoom.left;
		placeDoors();
	}
	else if (canGoRight()) {
		roomID = curRoom.right;
		placeDoors();
	}

	if (curRoom.type == ROOM_TYPE::BOSS && foundBoss == false) {
		foundBoss = true;
	}
	else if (curRoom.type == ROOM_TYPE::EXIT && foundBoss) {
		ret = true;
	}
	return ret;
}

void TestDemoScene::placeDoors()
{
	Room& curRoom = this->getComponent<Room>(rooms[roomID]);
	glm::vec3 curPos = this->getComponent<Transform>(rooms[roomID]).position;
	glm::vec3& camPos = this->getComponent<Transform>(this->getMainCameraID()).position;

	system("cls");
	std::cout << "Current Room: " << typeToString(curRoom.type) << " ID: " << roomID << std::endl << "Choises: " << std::endl;
	if (curRoom.down != -1) {
		std::cout << "down\n";
	}
	if (curRoom.left != -1) {
		std::cout << "right\n";
	}
	if (curRoom.right != -1) {
		std::cout << "left\n";
	}
	if (curRoom.up != -1) {
		std::cout << "up\n";
	}

	camPos = curPos;

	glm::vec3 posLeft = glm::vec3(-10000.0f, -10000.0f, -10000.0f);
	glm::vec3 posRight = glm::vec3(-10000.0f, -10000.0f, -10000.0f);
	glm::vec3 posUp = glm::vec3(-10000.0f, -10000.0f, -10000.0f);
	glm::vec3 posDown = glm::vec3(-10000.0f, -10000.0f, -10000.0f);
	if (curRoom.left != -1) {
		posLeft = glm::vec3(curPos.x - curRoom.dimensions.x / 2, 0.0f, curPos.z);
	}
	if (curRoom.right != -1) {
		posRight = glm::vec3(curPos.x + curRoom.dimensions.x / 2, 0.0f, curPos.z);
	}
	if (curRoom.up != -1) {
		posUp = glm::vec3(curPos.x, 0.0f, curPos.z + curRoom.dimensions.z/2);
	}
	if (curRoom.down != -1) {
		posDown = glm::vec3(curPos.x, 0.0f, curPos.z - curRoom.dimensions.z / 2);
	}
	glm::vec3& left = this->getComponent<Transform>(doors[0]).position;
	left = posLeft;
	glm::vec3& right = this->getComponent<Transform>(doors[1]).position;
	right = posRight;
	glm::vec3& up = this->getComponent<Transform>(doors[2]).position;
	up = posUp;
	glm::vec3& down = this->getComponent<Transform>(doors[3]).position;
	down = posDown;
}

bool TestDemoScene::canGoForward()
{
	glm::vec3 doorPos = this->getComponent<Transform>(doors[2]).position;
	glm::vec3 camPos = this->getComponent<Transform>(this->getMainCameraID()).position;
	bool ret = false;
	float radius = 10.0f;
	glm::vec3 diff = glm::abs(doorPos - camPos);
	if (diff.x <= radius && diff.y <= radius && diff.z <= radius) {
		ret = true;
	}
	return ret;
}

bool TestDemoScene::canGoBack()
{
	glm::vec3 doorPos = this->getComponent<Transform>(doors[3]).position;
	glm::vec3 camPos = this->getComponent<Transform>(this->getMainCameraID()).position;
	bool ret = false;
	float radius = 10.0f;
	glm::vec3 diff = glm::abs(doorPos - camPos);
	
	if (diff.x <= radius && diff.y <= radius && diff.z <= radius) {
		ret = true;
	}
	return ret;
}

bool TestDemoScene::canGoLeft()
{
	glm::vec3 doorPos = this->getComponent<Transform>(doors[0]).position;
	glm::vec3 camPos = this->getComponent<Transform>(this->getMainCameraID()).position;
	bool ret = false;
	float radius = 10.0f;
	glm::vec3 diff = glm::abs(doorPos - camPos);
	if (diff.x <= radius && diff.y <= radius && diff.z <= radius) {
		ret = true;
	}
	return ret;
}

bool TestDemoScene::canGoRight()
{
	glm::vec3 doorPos = this->getComponent<Transform>(doors[1]).position;
	glm::vec3 camPos = this->getComponent<Transform>(this->getMainCameraID()).position;
	bool ret = false;
	float radius = 10.0f;
	glm::vec3 diff = glm::abs(doorPos - camPos);
	if (diff.x <= radius && diff.y <= radius && diff.z <= radius) {
		ret = true;
	}
	return ret;
}
