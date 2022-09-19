#include <iostream>
#include "TestScene2.hpp"
#include "../application0/Input.hpp"
#include "../application0/Time.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "../components0/MeshComponent.hpp"

TestScene2::TestScene2()
{
}

TestScene2::~TestScene2()
{
}

void TestScene2::init()
{
	std::cout << "Test scene 2 init" << std::endl;

	// Create entity2 (already has transform)
	this->testEntity2 = this->createEntity();

	// Transform component
	Transform& transform2 = this->getComponent<Transform>(this->testEntity2);
	transform2.position = glm::vec3(20.F, 20.F, -100.F);
	transform2.rotation = glm::vec3(-90.0f, 40.0f, 0.0f);
	transform2.scale = glm::vec3(10.0f, 5.0f, 5.0f);

	// Mesh component
	this->setComponent<MeshComponent>(this->testEntity2);
	MeshComponent& meshComp2 = this->getComponent<MeshComponent>(this->testEntity2);
}

void TestScene2::update()
{
	if (Input::isMouseButtonPressed(Mouse::RIGHT))
	{
		std::cout << "(" << Input::getMouseX() << " " << 
			Input::getMouseY() << ")" << std::endl;
	}

	Transform& transform2 = this->getComponent<Transform>(this->testEntity2);
	transform2.position.x += Time::getDT();
}
