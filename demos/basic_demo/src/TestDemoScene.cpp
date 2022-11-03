#include <iostream>
#include "TestDemoScene.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "vengine.h"

#include "vengine/test/TestScene2.hpp"

TestDemoScene::TestDemoScene()
	: camEntity(-1), testEntity(-1)//, testEntity2(-1)
	, aniIDs{ -1, -1, -1, -1 }
	, aniActive{ true, true, true, true }
{
}

TestDemoScene::~TestDemoScene()
{
}
#include "vengine/VengineMath.hpp"
void TestDemoScene::init()
{	
	this->timer = 0.0f;

	// Camera
	this->camEntity = this->createEntity();
	this->setComponent<Camera>(this->camEntity);
	this->setMainCamera(this->camEntity);

	// Create entity (already has transform)
	this->testEntity = this->createEntity();

	// Transform component
	Transform& transform = this->getComponent<Transform>(this->testEntity);
	transform.position = glm::vec3(10.f, 0.f, 30.f);
	//transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
	this->setComponent<MeshComponent>(this->testEntity, (int)this->getResourceManager()->addMesh("assets/models/fine_ghost.obj"));
	this->setComponent<Collider>(this->testEntity, Collider::createCapsule(2.0f, 5.0f));
	this->setComponent<Rigidbody>(this->testEntity);
	this->getComponent<Rigidbody>(this->testEntity).rotFactor = glm::vec3(0.0f);
	this->setComponent<PointLight>(this->testEntity);
	this->getComponent<PointLight>(this->testEntity).color = glm::vec3(0.05f, 0.95f, 0.05f);

	// Create entity (already has transform)
	Entity ghostEntity2 = this->createEntity();

	// Transform component
	Transform& transform2 = this->getComponent<Transform>(ghostEntity2);
	transform2.position = glm::vec3(15.f, 0.f, 30.f);
	//transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
	this->setComponent<MeshComponent>(ghostEntity2, (int)this->getResourceManager()->addMesh("assets/models/fine_ghost.obj"));
	this->setComponent<Collider>(ghostEntity2, Collider::createCapsule(2.0f, 5.0f));
	this->setComponent<Rigidbody>(ghostEntity2);
	this->getComponent<Rigidbody>(ghostEntity2).rotFactor = glm::vec3(0.0f);
	this->setComponent<PointLight>(ghostEntity2);
	this->getComponent<PointLight>(ghostEntity2).color = glm::vec3(0.95f, 0.05f, 0.05f);

	// Create entity (already has transform)
	int puzzleTest = this->createEntity();
	this->setComponent<MeshComponent>(puzzleTest, (int)this->getResourceManager()->addMesh("assets/models/pussel1_5.fbx"));
	this->getComponent<Transform>(puzzleTest).rotation = glm::vec3(0, 180, 0);
	this->getComponent<Transform>(puzzleTest).position = glm::vec3(5, 0, 0);
	addCollisionToScene(this->getResourceManager()->getCollisionShapeFromMesh(this->getResourceManager()->addCollisionShapeFromMesh("assets/models/pussel1_5.fbx")), *this, glm::vec3(5, 0, 0), glm::vec3(0,180,0));
	addCollisionToNetworkScene(this->getResourceManager()->getCollisionShapeFromMesh(this->getResourceManager()->addCollisionShapeFromMesh("assets/models/pussel1_5.fbx")), nullptr, glm::vec3(5, 0, 0), glm::vec3(0,180,0));

	// Floor
	this->floor = this->createEntity();
	Transform& floorT = this->getComponent<Transform>(floor);
	floorT.position = glm::vec3(10.0f, -25.0f, 30.0f);
	floorT.scale = glm::vec3(100.0f, 1.0f, 100.0f);
	this->setComponent<MeshComponent>(floor, 0);
	this->setComponent<Collider>(floor, Collider::createBox(glm::vec3(100.0f, 1.0f, 100.0f)));

	// Create multiple test rigidbodies
	for (int x = 0; x < 5; x++)
	{
		for (int z = 0; z < 5; z++)
		{
			Entity e = this->createEntity();
			Transform& t = this->getComponent<Transform>(e);
			t.position = glm::vec3(x, 7.5f, z) * 10.0f;
			t.rotation = glm::vec3(rand() % 361, rand() % 361, rand() % 361);
			t.scale = glm::vec3((rand() % 101) * 0.01f + 1.5f);

			this->setComponent<Collider>(e, Collider::createBox(t.scale, rand() % 2));
			this->setComponent<Rigidbody>(e);
			this->setComponent<MeshComponent>(e, 0);
		}
	}

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

			newMeshComp.meshID = Scene::getResourceManager()->addMesh(
				"assets/models/Amogus/source/1.fbx");
			amogusMeshID = newMeshComp.meshID;
		}
		else
		{
			newTransform.position = glm::vec3(-7.f - i * 3.5f, -2.0f, 30.f);
			newTransform.rotation = glm::vec3(0.0f, 180.0f, 0.0f);
			newTransform.scale = glm::vec3(1.0f, 1.0f, 1.0f);

			/*newMeshComp.meshID = Scene::getResourceManager()->addMesh(
				"assets/models/run_forward_correct.fbx");*/
			newMeshComp.meshID = Scene::getResourceManager()->addMesh(
				"assets/models/Stormtrooper/source/silly_dancing.fbx",
				"assets/models/Stormtrooper/textures");
		}

		/*this->setComponent<AnimationComponent>(aniIDs[i]);
		AnimationComponent& newAnimComp = this->getComponent<AnimationComponent>(aniIDs[i]);
		newAnimComp.timer += 24.0f * 0.6f * i;
		newAnimComp.timeScale += i % 2;*/
	}
	// Output test
	Scene::getResourceManager()->getMesh(amogusMeshID).outputRigDebugInfo("skeletalAnimation.txt");

	Entity swarmEntity = this->createEntity();
	this->setComponent<MeshComponent>(swarmEntity);
	Transform& swarmTransform = this->getComponent<Transform>(swarmEntity);
	swarmTransform.position = glm::vec3(10.0f, 0.0f, 30.f);
	MeshComponent& swarmMesh = this->getComponent<MeshComponent>(swarmEntity);
	swarmMesh.meshID = Scene::getResourceManager()->addMesh(
		"assets/models/Swarm_Model.fbx",
		"assets/textures/swarmTextures");

	// Add textures for ui renderer
	TextureSamplerSettings samplerSettings{};
	samplerSettings.filterMode = vk::Filter::eNearest;
	this->uiTextureIndex0 = Scene::getResourceManager()->addTexture("assets/textures/test_UV.png", {samplerSettings});
	this->uiTextureIndex1 = Scene::getResourceManager()->addTexture("assets/textures/test_B.png", { samplerSettings });
	samplerSettings.unnormalizedCoordinates = VK_TRUE;
	this->fontTextureIndex = Scene::getResourceManager()->addTexture("assets/textures/testBitmapFont.png", { samplerSettings, true });

	Scene::getUIRenderer()->setBitmapFont(
		{
			"abcdefghij",
			"klmnopqrst",
			"uvwxyz+-.'",
			"0123456789",
			"!?,<>:()#^",
			"@         "
		},
		this->fontTextureIndex,
		16, 16
	);

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
	// Debug rendering on colliders
	this->getPhysicsEngine()->renderDebugShapes(Input::isKeyDown(Keys::Y));

	// Movement with velocity
	if (this->hasComponents<Collider, Rigidbody>(this->testEntity))
	{
		Rigidbody& rb = this->getComponent<Rigidbody>(this->testEntity);
		glm::vec3 vec = glm::vec3(Input::isKeyDown(Keys::LEFT) - Input::isKeyDown(Keys::RIGHT), 0.0f, Input::isKeyDown(Keys::UP) - Input::isKeyDown(Keys::DOWN));
		float y = rb.velocity.y;
		rb.velocity = vec * 10.0f;
		rb.velocity.y = y + Input::isKeyPressed(Keys::SPACE) * 5.0f;
	}

	// Test contact
	if (Input::isKeyDown(Keys::F))
	{
		Collider col = Collider::createBox(glm::vec3(100.0f, 1.0f, 100.0f));
		std::vector<Entity> hit = this->getPhysicsEngine()->testContact(col, glm::vec3(0.0f, 0.0f, 0.0f));
		for (const Entity& e : hit)
		{
			if ((int)rand() & 1)
			{
				this->setComponent<Inactive>(e);
			}
			else
			{
				this->getComponent<Rigidbody>(e).velocity.y += 5.0f;
			}
		}
	}

	// Test raycast
	if (Input::isKeyDown(Keys::R) && this->entityValid(this->getMainCameraID()))
	{
		Transform& camTransform = this->getComponent<Transform>(this->getMainCameraID());
		RayPayload payload = this->getPhysicsEngine()->raycast({ camTransform.position, camTransform.forward() });

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

	if (Input::isKeyPressed(Keys::T))
	{
		Scene::switchScene(new TestScene2(), "assets/scripts/scene.lua");
	}

	// UI
	Scene::getUIRenderer()->setTexture(this->uiTextureIndex0);
	Scene::getUIRenderer()->renderTexture(-960.0f, 540.0f, 200.0f, 200.0f);
	Scene::getUIRenderer()->renderTexture(-960.0f, -540.0f, 200.0f, 200.0f);
	Scene::getUIRenderer()->setTexture(this->uiTextureIndex1);
	Scene::getUIRenderer()->renderTexture(700.0f, 0.0f, 200.0f, 200.0f);
	Scene::getUIRenderer()->setTexture(this->fontTextureIndex);
	Scene::getUIRenderer()->renderString(
		"fps: " + std::to_string(1.0/Time::getDT()), 
		-400, 
		400, 
		50, 
		50,
		0,
		StringAlignment::LEFT
	);

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

void TestDemoScene::onCollisionStay(Entity e1, Entity e2)
{
	//Log::write("Collision Hit! " + std::to_string(e1) + " and " + std::to_string(e2));
}

void TestDemoScene::onTriggerStay(Entity e1, Entity e2)
{
	//Log::write("Trigger Hit! " + std::to_string(e1) + " and " + std::to_string(e2));
}