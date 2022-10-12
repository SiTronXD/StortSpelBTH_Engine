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
	: uiTextureIndex0(~0u),
	uiTextureIndex1(~0u)
{
}

TestScene2::~TestScene2()
{
}

void TestScene2::init()
{
	std::cout << "Test scene 2 init" << std::endl;

	// Add textures for ui renderer
	this->uiTextureIndex0 = Scene::getResourceManager()->addTexture("assets/textures/test_A.png");
	this->uiTextureIndex1 = Scene::getResourceManager()->addTexture("assets/textures/test_B.png");
}

void TestScene2::update()
{
	if (Input::isMouseButtonPressed(Mouse::RIGHT))
	{
		std::cout << "(" << Input::getMouseX() << " " << 
			Input::getMouseY() << ")" << std::endl;
	}

	Scene::getUIRenderer()->beginUI();
	Scene::getUIRenderer()->setTexture(this->uiTextureIndex0);
	Scene::getUIRenderer()->renderTexture(-0.25f, 0.0f, 1.0f, 1.0f);
	Scene::getUIRenderer()->renderTexture(-0.25f, -0.5f, 1.0f, 1.0f);
	Scene::getUIRenderer()->setTexture(this->uiTextureIndex1);
	Scene::getUIRenderer()->renderTexture(0.25f, 0.3f, 1.0f, 1.0f);
	Scene::getUIRenderer()->endUI();
}
