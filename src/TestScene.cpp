#include <iostream>
#include "TestScene.h"
#include "Input.h"

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
}

void TestScene::update()
{
	if (Input::isKeyDown(Keys::B))
	{
		std::cout << "B button!" << std::endl;
	}
}
