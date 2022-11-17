#include <iostream>
#include "TestDemoScene.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "vengine.h"
#include "vengine/test/TestScene2.hpp"
#include "vengine/VengineMath.hpp"

TestDemoScene::TestDemoScene()
	: camEntity(-1), testEntity(-1)//, testEntity2(-1)
	, aniIDs{ -1, -1, -1, -1 }
	, aniActive{ true, true, true, true }
{
}

TestDemoScene::~TestDemoScene()
{
}

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
	this->setComponent<Spotlight>(this->testEntity);
	this->getComponent<Spotlight>(this->testEntity).positionOffset = glm::vec3(0.0f, 5.0f, 0.0f);
	this->getComponent<Spotlight>(this->testEntity).direction = glm::vec3(0.0f, -1.0f, 0.0f); 
	this->getComponent<Spotlight>(this->testEntity).angle = 90.0f;
	this->getComponent<Spotlight>(this->testEntity).color = glm::vec3(0.02f, 0.95f, 0.02f) * 10.0f;

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
	this->getComponent<PointLight>(ghostEntity2).color = glm::vec3(0.95f, 0.05f, 0.05f) * 2.0f;

	// Ambient light
	Entity ambientLightEntity = this->createEntity();
	this->setComponent<AmbientLight>(ambientLightEntity);
	this->getComponent<AmbientLight>(ambientLightEntity).color = 
		glm::vec3(0.1f);

	// Directional light
	Entity directionalLightEntity = this->createEntity();
	this->setComponent<DirectionalLight>(directionalLightEntity);
	this->getComponent<DirectionalLight>(directionalLightEntity).color =
		glm::vec3(0.7);
	this->getComponent<DirectionalLight>(directionalLightEntity).direction =
		glm::vec3(-1.0f, -1.0f, 1.0f);

	// Create entity (already has transform)
	int puzzleTest = this->createEntity();
	this->setComponent<MeshComponent>(puzzleTest, (int)this->getResourceManager()->addMesh("assets/models/pussel1_5.fbx"));
	this->getComponent<Transform>(puzzleTest).rotation = glm::vec3(0, 180, 0);
	this->getComponent<Transform>(puzzleTest).position = glm::vec3(5, 0, 0);
	addCollisionToScene(this->getResourceManager()->getCollisionShapeFromMesh(this->getResourceManager()->addCollisionShapeFromMesh("assets/models/pussel1_5.fbx")), *this, glm::vec3(5, 0, 0), glm::vec3(0,180,0));
	addCollisionToNetworkScene(this->getResourceManager()->getCollisionShapeFromMesh(this->getResourceManager()->addCollisionShapeFromMesh("assets/models/pussel1_5.fbx")), glm::vec3(5, 0, 0), glm::vec3(0,180,0));

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

			this->setComponent<Collider>(e, Collider::createBox(t.scale, glm::vec3(0.0f, -5.0f, 0.0f), rand() % 2));
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
	/*int soundID = AudioHandler::loadFile("assets/sounds/test-audio.wav");
	if (soundID != -1)
	{
		this->setComponent<AudioSource>(this->testEntity);
		this->getComponent<AudioSource>(this->testEntity).sound.setBuffer(*AudioHandler::getBuffer(soundID));
	}*/

	// Create other test entities
	uint32_t audioId = this->getResourceManager()->addSound("assets/sounds/test-audio.wav");
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

			newMeshComp.meshID = Scene::getResourceManager()->addAnimations(
				{ "assets/models/Amogus/source/1.fbx" }, 
				"assets/models/Stormtrooper/textures"
			);
			amogusMeshID = newMeshComp.meshID;
		}
		else
		{
			newTransform.position = glm::vec3(-7.f - i * 3.5f, -2.0f, 30.f);
			newTransform.rotation = glm::vec3(0.0f, 180.0f, 0.0f);
			newTransform.scale = glm::vec3(1.0f, 1.0f, 1.0f);

			newMeshComp.meshID = Scene::getResourceManager()->addAnimations(
				{ "assets/models/Stormtrooper/source/silly_dancing.fbx" },
				"assets/models/Stormtrooper/textures"
			);
		}

		// Animation component
		this->setComponent<AnimationComponent>(aniIDs[i]);
		AnimationComponent& newAnimComp = this->getComponent<AnimationComponent>(aniIDs[i]);
		newAnimComp.timer += 24.0f * 0.6f * i;
		newAnimComp.timeScale += i % 2;
		newAnimComp.animationIndex = 0;

		// Make separate material
		if (i == 2)
		{
			this->getResourceManager()->makeUniqueMaterials(
				this->getComponent<MeshComponent>(aniIDs[i])
			);

			this->getComponent<MeshComponent>(aniIDs[i]).overrideMaterials[0].specularTextureIndex =
				this->getResourceManager()->addTexture("vengine_assets/textures/NoSpecular.png");
		}
	}
	
	Entity swarmEntity = this->createEntity();
	this->setComponent<MeshComponent>(swarmEntity);
	Transform& swarmTransform = this->getComponent<Transform>(swarmEntity);
	swarmTransform.position = glm::vec3(10.0f, 0.0f, 30.f);
	MeshComponent& swarmMesh = this->getComponent<MeshComponent>(swarmEntity);
	swarmMesh.meshID = Scene::getResourceManager()->addMesh(
		"assets/models/Swarm_Model.fbx",
		"assets/textures/swarmTextures");

	/*uint32_t meshId = Scene::getResourceManager()->addAnimations(
		{"assets/models/stickFirst.fbx", "assets/models/stickSecond.fbx", "assets/models/stickThird.fbx"});
	this->getResourceManager()->getMesh(meshId).mapAnimations(
		{"bendIdle", "fastBend", "dumb"});

	multiAnimation = this->createEntity();
	this->setComponent<MeshComponent>(multiAnimation);
	this->getComponent<MeshComponent>(multiAnimation).meshID = meshId;
	this->setComponent<AnimationComponent>(multiAnimation);
	this->getComponent<Transform>(multiAnimation).position.x = -30.f;
	this->getComponent<Transform>(multiAnimation).rotation.x = 90.f;*/


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
		glm::vec2(16, 16)
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

void TestDemoScene::start()
{
#if AUDIO
	uint32_t audioId = this->getResourceManager()->addSound("assets/sounds/test-audio.wav");

	audioSource1 = this->createEntity();
	this->setComponent<AudioSource>(audioSource1, audioId);
	this->setComponent<MeshComponent>(audioSource1, (int)this->getResourceManager()->addMesh("assets/models/fine_ghost.obj"));
	this->getComponent<Transform>(audioSource1).position.x = 30.f;
	volume1 = this->getComponent<AudioSource>(audioSource1).getVolume();

	audioSource2 = this->createEntity();
	this->setComponent<AudioSource>(audioSource2, audioId);
	this->setComponent<MeshComponent>(audioSource2, (int)this->getResourceManager()->addMesh("assets/models/fine_ghost.obj"));
	this->getComponent<Transform>(audioSource2).position.x = -30.f;
	volume2 = this->getComponent<AudioSource>(audioSource2).getVolume();

	this->getAudioHandler()->setMasterVolume(0.5f);
	master = this->getAudioHandler()->getMasterVolume();
	this->getAudioHandler()->setMusic("assets/sounds/notSusMusic.ogg");
	this->getAudioHandler()->playMusic();

	this->getAudioHandler()->setMusicVolume(music = 0.005f);

	//this->setComponent<AudioSource>(aniIDs[0], audioId);
	//this->getComponent<AudioSource>(aniIDs[0]).setVolume(0.2f);
	//this->getComponent<AudioSource>(aniIDs[0]).play();
	//this->getComponent<AudioSource>(aniIDs[0]).setLooping(true);
#endif
}

void TestDemoScene::update()
{
	// Rotate testEntity
	/*this->getComponent<Transform>(this->testEntity).rotation.x +=
		180.0f * Time::getDT(); */
	static float tim = 0.0f;
	tim += Time::getDT();
	this->getComponent<Spotlight>(this->testEntity).direction.x = std::sin(tim);
	this->getDebugRenderer()->renderSpotlight(this->testEntity);

	/*if (Input::isKeyReleased(Keys::ONE))
	{
		this->setAnimation(multiAnimation, "bendIdle");
	}
	else if (Input::isKeyReleased(Keys::TWO))
	{
		this->setAnimation(multiAnimation, "fastBend");
	}
	else if (Input::isKeyReleased(Keys::THREE))
	{
		this->setAnimation(multiAnimation, "dumb", false);
	}*/

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
		glm::vec3 rotVec = glm::vec3(Input::isKeyDown(Keys::I) - Input::isKeyDown(Keys::K), Input::isKeyDown(Keys::J) - Input::isKeyDown(Keys::L), 0.0f);
		Transform& camTransform = this->getComponent<Transform>(this->getMainCameraID());
		camTransform.position += (moveVec.x * camTransform.right() + glm::vec3(0.0f, moveVec.y, 0.0f) + moveVec.z * camTransform.forward()) * 25.0f * Time::getDT();
		camTransform.rotation += rotVec * 100.0f * Time::getDT();
	}

	if (Input::isKeyPressed(Keys::T))
	{
		Scene::switchScene(new TestScene2(), "assets/scripts/scene.lua");
	}

	// UI
	Scene::getUIRenderer()->setTexture(this->uiTextureIndex0);
	Scene::getUIRenderer()->renderTexture(glm::vec2(-960.0f, 540.0f), glm::vec2(200.0f));
	Scene::getUIRenderer()->renderTexture(glm::vec2(-960.0f, -540.0f), glm::vec2(200.0f));
	Scene::getUIRenderer()->setTexture(this->uiTextureIndex1);
	Scene::getUIRenderer()->renderTexture(glm::vec3(0.0f, -2.0f, 0.0f), glm::vec2(200.0f));
	Scene::getUIRenderer()->renderString(
		"fps: " + std::to_string(1.0 / Time::getDT()),
		glm::vec3(0.0f),
		glm::vec2(50.0f),
		0.0f,
		StringAlignment::CENTER,
		glm::vec4(1.0f, 0.0f, 0.0f, 0.5f)
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
#if AUDIO
	if (ImGui::Begin("Sound"))
	{
		ImGui::PushItemWidth(-100.f);

		if (this->hasComponents<AudioSource>(this->audioSource1))
		{
			AudioSource& source = this->getComponent<AudioSource>(this->audioSource1);
			static bool loop = false;

			if (ImGui::Button("Play 1"))
			{
				source.play();
			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Loop", &loop))
			{
				source.setLooping(loop);
			}
			if (ImGui::DragFloat("Source 1 volume", &volume1, 0.01f, 0.f, 1.f))
			{
				source.setVolume(volume1);
			}
		}
		if (this->hasComponents<AudioSource>(this->audioSource2))
		{
			AudioSource& source = this->getComponent<AudioSource>(this->audioSource2);
			static bool loop = false;

			if (ImGui::Button("Play 2"))
			{
				source.play();
			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Loop 2", &loop))
			{
				source.setLooping(loop);
			}
			if (ImGui::DragFloat("Source 2 volume", &volume2, 0.01f, 0.f, 1.f))
			{
				source.setVolume(volume2);
			}
		}
		
		if (ImGui::DragFloat("Music volume", &music, 0.01f, 0.f, 1.f))
		{
			this->getAudioHandler()->setMusicVolume(music);
		}
		if (ImGui::DragFloat("Master volume", &master, 0.01f, 0.f, 1.f))
		{
			this->getAudioHandler()->setMasterVolume(master);
		}

		ImGui::PopItemWidth();
	}
	ImGui::End();
#endif

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
		this->getNetworkHandler()->sendTCPDataToClient(TCPPacketEvent{ (int)NetworkEvent::START});
	}

	if (Input::isKeyReleased(Keys::V))
	{
		this->getSceneHandler()->getWindow()->close();
	}
}

void TestDemoScene::onCollisionEnter(Entity e1, Entity e2)
{
	
}

void TestDemoScene::onCollisionStay(Entity e1, Entity e2)
{
	
}

void TestDemoScene::onCollisionExit(Entity e1, Entity e2)
{
	
}

void TestDemoScene::onTriggerEnter(Entity e1, Entity e2)
{

}

void TestDemoScene::onTriggerStay(Entity e1, Entity e2)
{

}

void TestDemoScene::onTriggerExit(Entity e1, Entity e2)
{

}