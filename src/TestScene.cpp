#include "TestScene.h"
#include <iostream>

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
	std::cout << "Test scene update" << std::endl;
}

void TestScene::renderUI()
{
}
