#include <iostream>
#include "TestDemoScene.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"

TestDemoScene::TestDemoScene()
	: testEntity(-1), testEntity2(-1), physEngine()
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

	// Create entity (already has transform)
	this->testEntity = this->createEntity();

	// Transform component
	Transform& transform = this->getComponent<Transform>(this->testEntity);
	transform.position = glm::vec3(1000.f, 0.f, 30.f);
	transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
	transform.scale = glm::vec3(10.0f, 5.0f, 5.0f);

	// Mesh component
	this->setComponent<MeshComponent>(this->testEntity);
	MeshComponent& meshComp = this->getComponent<MeshComponent>(this->testEntity);


	// Create entity2 (already has transform)
	this->testEntity2 = this->createEntity();

	// Transform component
	Transform& transform2 = this->getComponent<Transform>(this->testEntity2);
	transform2.position = glm::vec3(0.f, 100.f, 30.f);
	transform2.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
	transform2.scale = glm::vec3(10.0f, 5.0f, 5.0f);

	// Mesh component
	this->setComponent<MeshComponent>(this->testEntity2);
	MeshComponent& meshComp2 = this->getComponent<MeshComponent>(this->testEntity2);

	physEngine.shootRay(glm::vec3(0, 0, -60), glm::vec3(0, 0, 60));
	physEngine.createSphereCol(glm::vec3(0.f, -20.f, 25.f), 10.f);
	physEngine.createCapsuleCol(transform2.position, glm::vec3(20.f, 20.f, 20.f), 1.f, transform2.rotation);
	physEngine.createBoxCol(glm::vec3(0.f, -100.f, 0.f), glm::vec3(200.f, 10.f, 200.f));
}

void TestDemoScene::update()
{
	Transform& transform2 = this->getComponent<Transform>(this->testEntity2);
	//transform2.position.x += Time::getDT();

	physEngine.update(*this, this->testEntity2);
	if (Input::isKeyDown(Keys::E))
	{
		physEngine.applyForce(glm::vec3(0, 100, 0));
	}
	else
	{
		physEngine.applyForce(glm::vec3(0, -50, 0));
	}

	if (this->entityValid(this->getMainCameraID()))
	{
		glm::vec3 moveVec = glm::vec3(Input::isKeyDown(Keys::A) - Input::isKeyDown(Keys::D), 0.0f, Input::isKeyDown(Keys::W) - Input::isKeyDown(Keys::S));
		Transform& camTransform = this->getComponent<Transform>(this->getMainCameraID());
		camTransform.position += moveVec * 25.0f * Time::getDT();
	}
}
