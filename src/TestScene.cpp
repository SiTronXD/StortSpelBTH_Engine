#include "TestScene.h"
#include <iostream>

#include "Transform.h"

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

	this->testEntity = this->createEntity();
	this->setComponent<Transform>(this->testEntity);

	Transform& transform = this->getComponent<Transform>(this->testEntity);
	std::cout << transform.position.x << " " << transform.position.y << "\n";
}

void TestScene::update()
{
	std::cout << "Test scene update" << std::endl;
}

void TestScene::renderUI()
{
}
