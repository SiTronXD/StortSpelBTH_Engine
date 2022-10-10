#include <iostream>
#include "TestScene2.hpp"
#include "../application/Input.hpp"
#include "../application/Time.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "../components/MeshComponent.hpp"
#include "../graphics/UIRenderer.hpp"

TestScene2::TestScene2()
{
}

TestScene2::~TestScene2()
{
}

void TestScene2::init()
{
	std::cout << "Test scene 2 init" << std::endl;
}

void TestScene2::update()
{
	if (Input::isMouseButtonPressed(Mouse::RIGHT))
	{
		std::cout << "(" << Input::getMouseX() << " " << 
			Input::getMouseY() << ")" << std::endl;
	}

	Scene::getUIRenderer()->beginUI();
	Scene::getUIRenderer()->renderTexture(0, 0, 100, 100);
	Scene::getUIRenderer()->renderTexture(150, 0, 100, 100);
	Scene::getUIRenderer()->endUI();
}
