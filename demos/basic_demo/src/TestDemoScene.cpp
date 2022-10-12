#include <iostream>
#include "TestDemoScene.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "vengine.h"

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
	//transform.scale = glm::vec3(10.0f, 5.0f, 5.0f);
	//transform.scale = glm::vec3(0.1f, .1f, .1f);

	// Mesh component
	//this->setComponent<MeshComponent>(this->testEntity);
	this->setComponent<MeshComponent>(this->testEntity);
	MeshComponent& meshComp = this->getComponent<MeshComponent>(this->testEntity);
	meshComp.meshID = Scene::getResourceManager()->addMesh("ghost.obj");

	// AudioSource component
	int soundID = AudioHandler::loadFile("assets/sounds/test-audio.wav");
	if (soundID != -1)
	{
		this->setComponent<AudioSource>(this->testEntity);
		this->getComponent<AudioSource>(this->testEntity).sound.setBuffer(*AudioHandler::getBuffer(soundID));
	}


	// Create other test entities
	for (uint32_t i = 0; i < 4; ++i)
	{
		int newTestEntity = this->createEntity();

		Transform& newTransform = this->getComponent<Transform>(newTestEntity);

		this->setComponent<MeshComponent>(newTestEntity);
		MeshComponent& newMeshComp = this->getComponent<MeshComponent>(newTestEntity);
		if (i <= 1)
		{
			newTransform.position = glm::vec3(-7.f - i * 3.5f, -2.0f, 30.f);
			newTransform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
			newTransform.scale = glm::vec3(0.03f, 0.03f, 0.03f);

			newMeshComp.meshID = Scene::getResourceManager()->addMesh("assets/models/Amogus/source/1.fbx");
		}
		else
		{
			newTransform.position = glm::vec3(-7.f - i * 3.5f, -2.0f, 30.f);
			//newTransform.rotation = glm::vec3(0.0f, 180.0f, 0.0f);
			newTransform.scale = glm::vec3(1.0f, 1.0f, 1.0f);

			newMeshComp.meshID = Scene::getResourceManager()->addMesh("assets/models/Stormtrooper/source/silly_dancing.fbx");
		}

		this->setComponent<AnimationComponent>(newTestEntity);
		AnimationComponent& newAnimComp = this->getComponent<AnimationComponent>(newTestEntity);
		newAnimComp.timer += 24.0f * 0.6f * i;
		newAnimComp.timeScale += i % 2;
	}

	/*memcpy(meshComp.filePath, "sponza.obj",sizeof(meshComp.filePath));
 
	// // Create entity2 (already has transform)
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
	box = { .pos = {0.f, -100.f, 0.f}, .halfExtents = {200.f, 10.f, 200.f} };*/
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

	if (ImGui::Begin("Sound"))
	{
		ImGui::PushItemWidth(-100.f);

		static float volume = 0.f, master = 0.f;
		static bool loop = false, hasListener = false;

		if (this->hasComponents<AudioSource>(this->testEntity))
		{
			AudioSource& source = this->getComponent<AudioSource>(this->testEntity);

			if (ImGui::Button("Play"))
			{
				source.sound.play();
			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Loop", &loop))
			{
				source.sound.setLoop(loop);
			}

			ImGui::DragFloat("Source volume", &volume, 1.f, 0.f, 100.f);

			source.sound.setVolume(volume);
		}


		if (ImGui::Checkbox("Listener", &hasListener))
		{
			if (hasListener) this->setComponent<AudioListener>(this->getMainCameraID());
			else this->removeComponent<AudioListener>(this->getMainCameraID());
		}
		ImGui::DragFloat("Master volume", &master, 1.f, 0.f, 100.f);

		if (this->hasComponents<AudioListener>(this->getMainCameraID()))
		{
			AudioListener& listener = this->getComponent<AudioListener>(this->getMainCameraID());

			listener.setVolume(master);	
		}

		ImGui::PopItemWidth();
	}
	ImGui::End();
}