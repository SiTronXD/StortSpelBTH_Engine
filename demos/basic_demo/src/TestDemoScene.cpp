#include <iostream>
#include "TestDemoScene.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "vengine.h"

TestDemoScene::TestDemoScene()
	: testEntity(-1)//, testEntity2(-1)
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
	transform.position = glm::vec3(0.f, 0.f, 30.f);
	transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
	//transform.scale = glm::vec3(10.0f, 5.0f, 5.0f);
	//transform.scale = glm::vec3(0.1f, .1f, .1f);

	// Mesh component
	this->setComponent<MeshComponent>(this->testEntity);
	MeshComponent& meshComp = this->getComponent<MeshComponent>(this->testEntity);
	meshComp.meshID = Scene::getResourceManager()->addMesh("ghost.obj");

	// Test entity 2
	int testEntity2 = this->createEntity();

	Transform& transform2 = this->getComponent<Transform>(testEntity2);
	transform2.position = glm::vec3(-7.f, 0.f, 30.f);
	transform2.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
	transform2.scale = glm::vec3(0.03f, 0.03f, 0.03f);

	this->setComponent<MeshComponent>(testEntity2);
	MeshComponent& meshComp2 = this->getComponent<MeshComponent>(testEntity2);
	meshComp2.meshID = Scene::getResourceManager()->addMesh("Amogus/source/1.fbx");
}

void TestDemoScene::update()
{
	//Transform& transform2 = this->getComponent<Transform>(this->testEntity2);
	//transform2.position.x += Time::getDT();

	if (this->entityValid(this->getMainCameraID()))
	{
		glm::vec3 moveVec = glm::vec3(Input::isKeyDown(Keys::A) - Input::isKeyDown(Keys::D), 0.0f, Input::isKeyDown(Keys::W) - Input::isKeyDown(Keys::S));
		Transform& camTransform = this->getComponent<Transform>(this->getMainCameraID());
		camTransform.position += moveVec * 25.0f * Time::getDT();
	}
}