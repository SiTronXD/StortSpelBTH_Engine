#include <iostream>
#include "TestDemoScene.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/glm.hpp"

TestDemoScene::TestDemoScene()
	: testEntity(-1), testEntity2(-1), testEntity3(-1), testEntity4(-1), physEngine()
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

	// Collider components
	this->testEntity3 = this->createEntity();

	this->setComponent<SphereCollider>(this->testEntity3);
	SphereCollider& sphere = this->getComponent<SphereCollider>(this->testEntity3);
	sphere = { .pos = {0.f, 0.f, 25.f}, .radius = 20.f };

	this->setComponent<CapsuleCollider>(this->testEntity2);
	CapsuleCollider& capsule = this->getComponent<CapsuleCollider>(this->testEntity2);
	capsule = { .pos = transform2.position, .rot = transform2.rotation, .height = 20.f, .radius = 10.f };

	this->setComponent<RigidBody>(this->testEntity2);
	RigidBody& rigidBody = this->getComponent<RigidBody>(this->testEntity2);
	rigidBody = { .pos = transform2.position, .rot = transform2.rotation };

	this->testEntity4 = this->createEntity();

	this->setComponent<BoxCollider>(this->testEntity4);
	BoxCollider& box = this->getComponent<BoxCollider>(this->testEntity4);
	box = { .pos = {0.f, -100.f, 0.f}, .halfExtents = {200.f, 10.f, 200.f} };
}

void TestDemoScene::update()
{
	Transform& transform2 = this->getComponent<Transform>(this->testEntity2);
	//transform2.position.x += Time::getDT();

	//if (Input::isKeyDown(Keys::E))
	//{
	//	physEngine.applyForce(glm::vec3(0, 100, 0));
	//}
	//else
	//{
	//	physEngine.applyForce(glm::vec3(0, -50, 0));
	//}
	if (Input::isKeyDown(Keys::T))
	{
		physEngine.shootRay(glm::vec3(0, 0, -60), glm::vec3(0, 0, 60));
	}

	if (this->entityValid(this->getMainCameraID()))
	{
		glm::vec3 moveVec = glm::vec3(Input::isKeyDown(Keys::A) - Input::isKeyDown(Keys::D), 0.0f, Input::isKeyDown(Keys::W) - Input::isKeyDown(Keys::S));
		Transform& camTransform = this->getComponent<Transform>(this->getMainCameraID());
		camTransform.position += moveVec * 25.0f * Time::getDT();
	}
}
