#include <iostream>
#include "TestDemoScene.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "vengine.h"

#include "vengine/test/TestScene2.hpp"

TestDemoScene::TestDemoScene()
	: testEntity(-1)//, testEntity2(-1)
	, aniIDs{-1, -1, -1, -1}
	, aniActive{true, true, true, true}
{
}

TestDemoScene::~TestDemoScene()
{
}

void TestDemoScene::init()
{
	std::cout << "Test scene init" << std::endl;    

	this->timer = 0.0f;

	// Camera
	Entity camEntity = this->createEntity();
	this->setComponent<Camera>(camEntity);
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
	uint32_t amogusMeshID = ~0u;
	for (uint32_t i = 0; i < 4; ++i)
	{
		aniIDs[i] = this->createEntity();

		Transform& newTransform = this->getComponent<Transform>(aniIDs[i]);

		this->setComponent<MeshComponent>(aniIDs[i]);
		MeshComponent& newMeshComp = this->getComponent<MeshComponent>(aniIDs[i]);
		if (i <= 1)
		{
			newTransform.position = glm::vec3(-7.f - i * 3.5f, -2.0f, 30.f);
			newTransform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
			newTransform.scale = glm::vec3(0.03f, 0.03f, 0.03f);

			newMeshComp.meshID = Scene::getResourceManager()->addMesh("assets/models/Amogus/source/1.fbx");
			amogusMeshID = newMeshComp.meshID;
		}
		else
		{
			newTransform.position = glm::vec3(-7.f - i * 3.5f, -2.0f, 30.f);
			newTransform.rotation = glm::vec3(0.0f, 180.0f, 0.0f);
			newTransform.scale = glm::vec3(1.0f, 1.0f, 1.0f);

			newMeshComp.meshID = Scene::getResourceManager()->addMesh("assets/models/Stormtrooper/source/silly_dancing.fbx");
		}

		this->setComponent<AnimationComponent>(aniIDs[i]);
		AnimationComponent& newAnimComp = this->getComponent<AnimationComponent>(aniIDs[i]);
		newAnimComp.timer += 24.0f * 0.6f * i;
		newAnimComp.timeScale += i % 2;
	}
	// Output test
	Scene::getResourceManager()->getMesh(amogusMeshID).outputRigDebugInfo("skeletalAnimation.txt");

	// Add textures for ui renderer
	TextureSamplerSettings samplerSettings{};
	samplerSettings.filterMode = vk::Filter::eNearest;
	this->uiTextureIndex0 = Scene::getResourceManager()->addTexture("assets/textures/test_UV.png", samplerSettings);
	this->uiTextureIndex1 = Scene::getResourceManager()->addTexture("assets/textures/test_B.png", samplerSettings);

	/*memcpy(meshComp.filePath, "sponza.obj",sizeof(meshComp.filePath));
 
	// // Create entity2 (already has transform)
	this->testEntity2 = this->createEntity();

	// Transform component
	Transform& transform2 = this->getComponent<Transform>(this->testEntity2);
	transform2.position = glm::vec3(0.f, 0.f, 20.f);
	transform2.rotation = glm::vec3(-90.0f, 40.0f, 0.0f);
	transform2.scale = glm::vec3(10.0f, 10.0f, 10.0f);

	// Mesh component
	this->setComponent<MeshComponent>(this->testEntity2);
	MeshComponent& meshComp2 = this->getComponent<MeshComponent>(this->testEntity2);*/
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

	if (Input::isKeyPressed(Keys::T))
	{
		Scene::switchScene(new TestScene2(), "assets/scripts/scene.lua");
	}

	// UI
	Scene::getUIRenderer()->setTexture(this->uiTextureIndex0);
	Scene::getUIRenderer()->renderTexture(-960.0f,  540.0f, 200.0f, 200.0f);
	Scene::getUIRenderer()->renderTexture(-960.0f, -540.0f, 200.0f, 200.0f);
	Scene::getUIRenderer()->setTexture(this->uiTextureIndex1);
	Scene::getUIRenderer()->renderTexture(700.0f, 0.0f, 200.0f, 200.0f);

	// Debug rendering

	// Lines
	Scene::getDebugRenderer()->renderLine(
		glm::vec3(-10.0f + 20.0f * std::sin(this->timer), -10.0f, 35.0f),
		glm::vec3(10.0f, 10.0f, 25.0f),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);
	Scene::getDebugRenderer()->renderLine(
		glm::vec3(0.0f, -10.0f, 35.0f),
		glm::vec3(0.0f + 20.0f * std::sin(this->timer + 5.15f), 10.0f, 25.0f),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);

	// Spheres
	Scene::getDebugRenderer()->renderSphere(
		glm::vec3(0.0f, 0.0f, 30.0f),
		1.0f,
		glm::vec3(1.0f, 1.0f, 0.0f)
	);
	Scene::getDebugRenderer()->renderSphere(
		glm::vec3(3.0f, 0.0f, 30.0f),
		2.0f,
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	// Boxes
	Scene::getDebugRenderer()->renderBox(
		glm::vec3(5.5f, 0.0f, 30.0f),
		glm::vec3(timer * 30.0f, timer * 30.0f * 2.541f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);
	Scene::getDebugRenderer()->renderBox(
		glm::vec3(6.25f, 0.0f, 30.0f),
		glm::vec3(timer * 30.0f, timer * 30.0f * 3.541f, 0.0f),
		glm::vec3(0.5f, 2.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 1.0f)
	);

	// Capsules
	Scene::getDebugRenderer()->renderCapsule(
		glm::vec3(8.0f, 0.0f, 30.0f),
		glm::vec3(timer * 30.0f, timer * 30.0f * 3.541f, 0.0f),
		2.0f + sin(timer),
		0.5f,
		glm::vec3(1.0f, 0.0f, 0.0f)
	);

	// Skeleton
	Scene::getDebugRenderer()->renderSkeleton(
		this->aniIDs[2],
		glm::vec3(1.0f, 1.0f, 0.0f)
	);

	this->timer += Time::getDT();

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

	if (ImGui::Begin("Set Active"))
	{
		for (int i = 0; i < 4; i++)
		{
			if (ImGui::Checkbox(("Animated entity: " + std::to_string(i)).c_str(), &aniActive[i]))
			{
				if (aniActive[i]) this->setActive(aniIDs[i]);
				else this->setInactive(aniIDs[i]);
			}
		}

		static bool entityOne = true, mainCamera = true;
		if (ImGui::Checkbox("testEntity", &entityOne))
		{
			if (entityOne) this->setActive(testEntity);
			else this->setInactive(testEntity);
		}

		if (ImGui::Checkbox("Main Camera", &mainCamera))
		{
			if (mainCamera) this->setActive(this->getMainCameraID());
			else this->setInactive(this->getMainCameraID());
		}

	}
	ImGui::End();

	if (Input::isKeyPressed(Keys::O)) {
		this->getNetworkHandler()->createServer();
		this->getNetworkHandler()->createClient();
		this->getNetworkHandler()->connectClientToThis();
	}
	if (Input::isKeyPressed(Keys::I)) {
		this->getNetworkHandler()->sendTCPDataToClient(TCPPacketEvent{ GameEvents::START });
	}
}