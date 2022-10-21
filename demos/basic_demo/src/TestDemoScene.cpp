#include <iostream>
#include "TestDemoScene.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "vengine.h"

TestDemoScene::TestDemoScene()
	: testEntity(-1), testEntity2(-1), testEntity3(-1), testEntity4(-1)
{
}

TestDemoScene::~TestDemoScene()
{
}

void TestDemoScene::init()
{
	std::cout << "Test scene init" << std::endl;    

	//  Camera
	int camEntity = this->createEntity();
	this->setComponent<Camera>(camEntity, 1.0f);
	this->setMainCamera(camEntity);

	//  Create entity (already has transform)
	this->testEntity = this->createEntity();

	//  Transform component
	Transform& transform = this->getComponent<Transform>(this->testEntity);
	transform.position = glm::vec3(10.f, 0.f, 30.f);
	transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
	this->setComponent<MeshComponent>(this->testEntity, (int)this->getResourceManager()->addMesh("assets/models/fine_ghost.obj"));
	this->setComponent<Collider>(this->testEntity, Collider::createSphere(5.0f));
	this->setComponent<Rigidbody>(this->testEntity, 1.0f, 1.0f);

	// Floor
	this->floor = this->createEntity();
	Transform& floorT = this->getComponent<Transform>(floor);
	floorT.position = glm::vec3(10.0f, -25.0f, 30.0f);
	floorT.scale = glm::vec3(100.0f, 1.0f, 100.0f);
	this->setComponent<MeshComponent>(floor, 0);
	this->setComponent<Collider>(floor, Collider::createBox(glm::vec3(100.0f, 1.0f, 100.0f)));

	//Rigidbody& rb = this->getComponent<Rigidbody>(this->testEntity);

	// transform.scale = glm::vec3(10.0f, 5.0f, 5.0f);
	// transform.scale = glm::vec3(0.1f, .1f, .1f);
	//this->setComponent<RigidBody>(this->testEntity);
	//this->setComponent<CapsuleCollider>(this->testEntity);

	//  Mesh component
	// this->setComponent<MeshComponent>(this->testEntity);
	// MeshComponent& meshComp = this->getComponent<MeshComponent>(this->testEntity);
	// meshComp.meshID = this->getResourceManager()->addMesh("ghost.obj");

	//  AudioSource component
	int soundID = AudioHandler::loadFile("assets/sounds/test-audio.wav");
	if (soundID != -1)
	{
		this->setComponent<AudioSource>(this->testEntity);
		this->getComponent<AudioSource>(this->testEntity).sound.setBuffer(*AudioHandler::getBuffer(soundID));
	}

	//  Create other test entities
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
			// newTransform.rotation = glm::vec3(0.0f, 180.0f, 0.0f);
			newTransform.scale = glm::vec3(1.0f, 1.0f, 1.0f);

			newMeshComp.meshID = Scene::getResourceManager()->addMesh("assets/models/Stormtrooper/source/silly_dancing.fbx");
		}

		this->setComponent<AnimationComponent>(newTestEntity);
		AnimationComponent& newAnimComp = this->getComponent<AnimationComponent>(newTestEntity);
		newAnimComp.timer += 24.0f * 0.6f * i;
		newAnimComp.timeScale += i % 2;
	}
}

void TestDemoScene::update()
{
	// Transform& transform2 = this->getComponent<Transform>(this->testEntity2);
	// transform2.position.x += Time::getDT();

	/*Transform& floorT = this->getComponent<Transform>(this->floor);
	floorT.rotation.z += (this->rotDir * 2 - 1) * 25.0f * Time::getDT();
	if (abs(floorT.rotation.z) > 20.0f) { this->rotDir = !this->rotDir; }*/

	Rigidbody& rb = this->getComponent<Rigidbody>(this->testEntity);
	glm::vec3 mVec = glm::vec3(Input::isKeyDown(Keys::LEFT) - Input::isKeyDown(Keys::RIGHT), 0, Input::isKeyDown(Keys::UP) - Input::isKeyDown(Keys::DOWN));
	rb.velocity = mVec * 5.0f * Time::getDT();
	if (Input::isKeyDown(Keys::R))
	{
		rb.acceleration = glm::vec3(0.0f, 25.0f, 0.0f);
	}
	if (Input::isKeyPressed(Keys::F))
	{
		Collider& col = this->getComponent<Collider>(this->floor);
		col.isTrigger = !col.isTrigger;
	}

	if (Input::isKeyDown(Keys::T) && this->entityValid(this->getMainCameraID()))
	{
		Transform& camTransform = this->getComponent<Transform>(this->getMainCameraID());
		RayPayload payload = this->getPhysicsEngine()->shootRay(Ray{ camTransform.position, camTransform.forward() });

		if (payload.hit)
		{
			Log::write("Hit raycast at pos (" +
				std::to_string(payload.hitPoint.x) + ", " +
				std::to_string(payload.hitPoint.y) + ", " +
				std::to_string(payload.hitPoint.z) + ")");
		}
	}

	if (this->entityValid(this->getMainCameraID()))
	{
		glm::vec3 moveVec = glm::vec3(Input::isKeyDown(Keys::A) - Input::isKeyDown(Keys::D), Input::isKeyDown(Keys::Q) - Input::isKeyDown(Keys::E), Input::isKeyDown(Keys::W) - Input::isKeyDown(Keys::S));
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