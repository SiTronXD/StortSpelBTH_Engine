#include "TestScene.h"

TestScene::TestScene(SceneHandler& sceneHandler)
	: Scene(sceneHandler)
{
}

TestScene::~TestScene()
{
}
#include <iostream>
void TestScene::init()
{
	std::cout << "Test scene init" << std::endl;
}

void TestScene::update()
{
	//std::cout << "Test scene update" << std::endl;
}

void TestScene::renderUI()
{
}
