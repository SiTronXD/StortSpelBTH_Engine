#include <iostream>
#include "TestScene.h"
#include "Input.h"
#include "glm/gtx/string_cast.hpp"

TestScene::TestScene(SceneHandler& sceneHandler)
	: Scene(sceneHandler)
{
}

TestScene::~TestScene()
{
}

void TestScene::init()
{
	std::cout << "Test scene init" << std::endl;

	// Testing component functionality
	this->testEntity = this->createEntity();
	this->setComponent<Transform>(this->testEntity);

	Transform& transform = this->getComponent<Transform>(this->testEntity);
	std::cout << "Transform: " << glm::to_string(transform.position) << "\n";
	transform.position = glm::vec3(1.0f);
	std::cout << "Transform: " << glm::to_string(transform.position) << "\n";
}

void TestScene::update()
{
	if (Input::isKeyDown(Keys::B))
	{
		std::cout << "B button!" << std::endl;
	}
}
