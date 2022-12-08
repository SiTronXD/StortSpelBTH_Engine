#include <iostream>
#include "TestDemoScene.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "vengine.h"
#include "vengine/test/TestScene2.hpp"
#include "vengine/VengineMath.hpp"

TestDemoScene::TestDemoScene()
	: camEntity(-1), testEntity(-1), testEntity2(-1)
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
	this->setComponent<NoShadowCasting>(this->testEntity);

	// Create entity (already has transform)
	this->testEntity2 = this->createEntity();

	// Transform component
	Transform& transform2 = this->getComponent<Transform>(this->testEntity2);
	transform2.position = glm::vec3(15.f, 0.f, 30.f);
	//transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
	this->setComponent<MeshComponent>(this->testEntity2, (int)this->getResourceManager()->addMesh("assets/models/fine_ghost.obj"));
	this->setComponent<Collider>(this->testEntity2, Collider::createCapsule(2.0f, 5.0f));
	this->setComponent<Rigidbody>(this->testEntity2);
	this->getComponent<Rigidbody>(this->testEntity2).rotFactor = glm::vec3(0.0f);
	this->setComponent<PointLight>(this->testEntity2);
	this->getComponent<PointLight>(this->testEntity2).color = glm::vec3(0.95f, 0.05f, 0.05f) * 20.0f;

	// Ambient light
	Entity ambientLightEntity = this->createEntity();
	this->setComponent<AmbientLight>(ambientLightEntity);
	this->getComponent<AmbientLight>(ambientLightEntity).color = 
		glm::vec3(0.05f);

	// Directional light
	this->directionalLightEntity = this->createEntity();
	this->setComponent<DirectionalLight>(this->directionalLightEntity);
	this->getComponent<DirectionalLight>(this->directionalLightEntity)
		.color = glm::vec3(0.5);
	this->getComponent<DirectionalLight>(this->directionalLightEntity)
		.direction = glm::vec3(-1.0f, -1.0f, 1.0f);
	this->getComponent<DirectionalLight>(this->directionalLightEntity)
		.cascadeDepthScale = 5.0f;

	// Particle system
	this->particleSystemEntities.push_back(this->createEntity());
	this->setComponent<ParticleSystem>(this->particleSystemEntities[0]);
	ParticleSystem& partSys = this->getComponent<ParticleSystem>(this->particleSystemEntities[0]);
	partSys.maxlifeTime = 4.0f;
	partSys.numParticles = 512;
	partSys.textureIndex = this->getResourceManager()->addTexture("vengine_assets/textures/me.png");
	partSys.startSize = glm::vec2(0.5f);
	partSys.endSize = glm::vec2(0.0f);
	partSys.startColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	partSys.endColor = glm::vec4(0.2f, 0.2f, 0.2f, 0.0f);
	partSys.velocityStrength = 17.0f;
	partSys.acceleration = glm::vec3(0.0f, -13.0f, 0.0f);
	//partSys.initialSimulation = true;

	partSys.coneSpawnVolume.diskRadius = 0.0f;
	partSys.coneSpawnVolume.coneAngle = 30.0f;
	partSys.coneSpawnVolume.localDirection = glm::vec3(0.0f, -1.0f, -1.0f);
	partSys.coneSpawnVolume.localPosition.x -= 40.0f;

	this->particleSystemEntities.push_back(this->createEntity());
	this->setComponent<ParticleSystem>(this->particleSystemEntities[1]);
	ParticleSystem& partSys1 = this->getComponent<ParticleSystem>(this->particleSystemEntities[1]);
	partSys1.maxlifeTime = 3.0f;
	partSys1.numParticles = 512;
	partSys1.textureIndex = this->getResourceManager()->addTexture("vengine_assets/textures/me.png");
	partSys1.startSize = glm::vec2(1.0f);
	partSys1.endSize = glm::vec2(0.1f);
	partSys1.startColor = glm::vec4(1.0f);
	partSys1.endColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
	partSys1.velocityStrength = 10.0f;
	partSys1.acceleration = glm::vec3(0.0f, -13.0f, 0.0f);
	
	partSys1.coneSpawnVolume.diskRadius = 0.0f;
	partSys1.coneSpawnVolume.coneAngle = 60.0f;
	partSys1.coneSpawnVolume.localDirection = glm::vec3(0.0f, -1.0f, -1.0f);
	partSys1.coneSpawnVolume.localPosition.x -= 20.0f;

	partSys1.respawnSetting = RespawnSetting::EXPLOSION;
	partSys1.spawn = false;


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
			t.position = glm::vec3(x, 2.5f, z) * 10.0f;
			t.rotation = glm::vec3(rand() % 361, rand() % 361, rand() % 361);
			t.scale = glm::vec3((rand() % 101) * 0.01f + 1.5f);

			this->setComponent<Collider>(e, Collider::createBox(t.scale, glm::vec3(0.0f, -5.0f, 0.0f), rand() % 2));
			this->setComponent<Rigidbody>(e);
			this->setComponent<MeshComponent>(e, 0);
		}
	}

#if 0
	int playerMesh = Scene::getResourceManager()->addAnimations({ "assets/models/char/cRun.fbx","assets/models/char/cIdle.fbx", "assets/models/char/cAttack2.fbx" });
	Scene::getResourceManager()->mapAnimations(playerMesh, {"run", "idle", "attack"});
	getResourceManager()->getMesh(playerMesh).createAnimationSlot("LowerBody", "mixamorig:Hips");
	getResourceManager()->getMesh(playerMesh).createAnimationSlot("UpperBody", "mixamorig:Spine1");
	
	multiAni2 = createEntity();
	setComponent<MeshComponent>(multiAni2, playerMesh);
	setComponent<AnimationComponent>(multiAni2);
	
	getComponent<Transform>(multiAni2).position.x = 40.f;
	getComponent<Transform>(multiAni2).rotation.y = 180.f;
#endif

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
	uint32_t amogusMeshID= Scene::getResourceManager()->addAnimations(
			{ "assets/models/Amogus/source/1.fbx","assets/models/Amogus/source/2.fbx" }, 
			"assets/models/Stormtrooper/textures");
	getResourceManager()->mapAnimations(amogusMeshID, {"Run", "Idk"});
	Mesh& amogMesh = getResourceManager()->getMesh(amogusMeshID);
	amogMesh.createAnimationSlot("mainSlot", "Tors");
	amogMesh.createAnimationSlot("LeftLegSlot", "Left_leg");
	amogMesh.createAnimationSlot("RightLegSlot", "Right_leg");

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
			newMeshComp.meshID = amogusMeshID;
		}
		else
		{
			newTransform.position = glm::vec3(-7.f - i * 3.5f, -2.0f, 30.f);
			newTransform.rotation = glm::vec3(0.0f, 180.0f, 0.0f);
			newTransform.scale = glm::vec3(1.0f, 1.0f, 1.0f);

			newMeshComp.meshID = Scene::getResourceManager()->addAnimations(
				{ "assets/models/Stormtrooper/source/silly_dancing.fbx" },
				"assets/models/Stormtrooper/textures");
		}

		// Animation component
		this->setComponent<AnimationComponent>(aniIDs[i]);
		AnimationComponent& newAnimComp = this->getComponent<AnimationComponent>(aniIDs[i]);
		newAnimComp.aniSlots[0].timer += 24.0f * 0.6f * i;
		newAnimComp.aniSlots[0].timeScale += i % 2;
		newAnimComp.aniSlots[0].animationIndex = 0;
    
		MeshComponent& meshComp =
			this->getComponent<MeshComponent>(this->aniIDs[i]);
		if (i == 2)
		{
			this->getResourceManager()->makeUniqueMaterials(meshComp);
			meshComp.overrideMaterials[0].glowMapTextureIndex =
				this->getResourceManager()->addTexture(
					"vengine_assets/models/Stormtrooper/textures/Stormtrooper_D_Specular.png"
				);
		}
		else if(i == 3)
		{
			this->getResourceManager()->getMaterial(meshComp, 0).glowMapTextureIndex = 
				this->getResourceManager()->addTexture(
					"vengine_assets/textures/White.png"
				);
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
	audioBuffer1 = this->getResourceManager()->addSound("assets/sounds/test-audio.wav");
	audioBuffer2 = this->getResourceManager()->addSound("assets/sounds/OufSound.ogg");

	multiAudio = createEntity();
	this->getAudioHandler()->requestAudioSource(multiAudio, audioBuffer1);
	getComponent<AudioSource>(multiAudio).play();
	for (int i = 0; i < 16; i++)
	{
		audioSources[i] = createEntity();
		getComponent<Transform>(audioSources[i]).position.x = i * 50.f;
		//this->getAudioHandler()->playSound(audioSources[i], audioBuffer1);
	}
	



	this->getAudioHandler()->setMasterVolume(0.5f);
	master = this->getAudioHandler()->getMasterVolume();
	this->getAudioHandler()->setMusic("assets/sounds/notSusMusic.ogg");
	this->getAudioHandler()->playMusic();

	this->getAudioHandler()->setMusicVolume(music = 0.01f);
}

void TestDemoScene::update()
{
	// Fog
	float startDist = this->getFogSettings().fogStartDist;
	float absorption = this->getFogSettings().fogAbsorption;
	ImGui::Begin("Fog");
	ImGui::SliderFloat("Start distance: ", &startDist, -100.0f, 100.0f);
	ImGui::SliderFloat("Absorption: ", &absorption, 0.0f, 1.0f);
	ImGui::End();
	this->setFogStartDistance(startDist);
	this->setFogAbsorption(absorption);

	// Particle system
	ImGui::Begin("Particle System");
	for (size_t i = 0; i < this->particleSystemEntities.size(); ++i)
	{
		Transform& partSysTran = this->getComponent<Transform>(this->particleSystemEntities[i]);
		ParticleSystem& partSys = this->getComponent<ParticleSystem>(this->particleSystemEntities[i]);
		ImGui::SliderFloat3(("Entity position " + std::to_string(i) + ": ").c_str(), &partSysTran.position[0], -5.0f, 5.0f);
		ImGui::SliderFloat3(("Entity rotation " + std::to_string(i) + ": ").c_str(), &partSysTran.rotation[0], -180.0f, 180.0f);
		ImGui::SliderFloat3(("Cone pos " + std::to_string(i) + ": ").c_str(), &partSys.coneSpawnVolume.localPosition[0], -5.0f, 5.0f);
		ImGui::SliderFloat3(("Cone dir " + std::to_string(i) + ": ").c_str(), &partSys.coneSpawnVolume.localDirection[0], -1.0f, 1.0f);
		ImGui::SliderFloat(("Disk radius " + std::to_string(i) + ": ").c_str(), &partSys.coneSpawnVolume.diskRadius, 0.0f, 10.0f);
		ImGui::SliderFloat(("Cone angle " + std::to_string(i) + ": ").c_str(), &partSys.coneSpawnVolume.coneAngle, 0.0f, 180.0f);
		ImGui::SliderFloat(("Velocity strength: " + std::to_string(i) + ": ").c_str(), &partSys.velocityStrength, 0.0f, 20.0f);
		ImGui::SliderFloat(("Spawn rate: " + std::to_string(i) + ": ").c_str(), &partSys.spawnRate, 0.0f, 1.0f);
		ImGui::Checkbox(("Spawn: " + std::to_string(i) + ": ").c_str(), &partSys.spawn);
	}
	ImGui::End();

	// Make ghosts follow skeletal animation
	this->getComponent<Transform>(this->testEntity).setMatrix(
		this->getResourceManager()->getJointTransform(
			this->getComponent<Transform>(this->aniIDs[1]),
			this->getComponent<MeshComponent>(this->aniIDs[1]),
			this->getComponent<AnimationComponent>(this->aniIDs[1]),
			"Left_leg"
		) * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f))
	);
	this->getComponent<Transform>(this->testEntity2).setMatrix(
		this->getResourceManager()->getJointTransform(
			this->getComponent<Transform>(this->aniIDs[1]),
			this->getComponent<MeshComponent>(this->aniIDs[1]),
			this->getComponent<AnimationComponent>(this->aniIDs[1]),
			"Right_leg"
		) * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f))
	);

	if (Input::isKeyReleased(Keys::ONE))
	{
		setAnimation(aniIDs[0], "Idk", "mainSlot");
		setAnimation(aniIDs[1], "Idk");
	}
	else if (Input::isKeyReleased(Keys::TWO))
	{
		setAnimation(aniIDs[0], "Run", "LeftLegSlot");
		setAnimation(aniIDs[1], "Run");
	}
	else if (Input::isKeyReleased(Keys::THREE))
	{
		setAnimation(aniIDs[0], "Idk", "RightLegSlot");
	}
	else if (Input::isKeyReleased(Keys::FOUR))
	{
		setAnimation(aniIDs[0], "Run");
	}

#if 0
	if (ImGui::Begin("Animation Blending"))
	{
		AnimationComponent& aniComp = this->getComponent<AnimationComponent>(multiAni2);
		MeshComponent& meshComp = this->getComponent<MeshComponent>(multiAni2);

		ImGui::PushItemWidth(-110.f);

		static int slotIdx = 0;
		ImGui::Text("Slot:");
		ImGui::RadioButton("Lower body", &slotIdx, 0); ImGui::SameLine();
		ImGui::RadioButton("Upper body", &slotIdx, 1); ImGui::SameLine();
		ImGui::RadioButton("Full body", &slotIdx, -1);
		int usedIdx = slotIdx != -1 ? slotIdx : 0;

		if (slotIdx == -1)
		{
			aniComp.aniSlots[1] = aniComp.aniSlots[0];
		}

		ImGui::DragFloat("Tranition Time", &aniComp.aniSlots[usedIdx].transitionTime, 0.01f, 0.f, 5.f);
		ImGui::Text("Alpha: %.2f", aniComp.aniSlots[usedIdx].alpha); 
		ImGui::SameLine();
		if (ImGui::Button("Sync slots"))
		{
			aniComp.aniSlots[1] = aniComp.aniSlots[0];
		}

		ImGui::Separator();

		ImGui::Text("Current Animation");
		ImGui::Text("Timer: %.2f ", aniComp.aniSlots[usedIdx].timer);
		ImGui::DragFloat("Time Scale", &aniComp.aniSlots[usedIdx].timeScale, 0.01f, 0.f, 10.f);
		ImGui::InputInt("Ani Idx", &aniComp.aniSlots[usedIdx].animationIndex, 1, 1);
		aniComp.aniSlots[usedIdx].animationIndex = std::clamp(aniComp.aniSlots[usedIdx].animationIndex, 0, 2);
		

		ImGui::Separator();

		ImGui::Text("Next Animation");
		ImGui::Text("Timer: %.2f", aniComp.aniSlots[usedIdx].nTimer);
		ImGui::DragFloat("Time Scale ##0", &aniComp.aniSlots[usedIdx].nTimeScale, 0.01f, 0.f, 10.f);
		static int nextIdx = 0;
		ImGui::InputInt("Ani Idx ##0", &nextIdx, 1, 1);
		nextIdx = std::clamp(nextIdx, 0, 2);

		if (ImGui::Button("Start Transition"))
		{
			std::string aniName = nextIdx == 0 ? "run" : nextIdx == 1 ? "idle" : "attack";
			std::string slotName = slotIdx == 0 ? "LowerBody" : slotIdx == 1 ? "UpperBody" : "";
			blendToAnimation(multiAni2, aniName, slotName, aniComp.aniSlots[usedIdx].transitionTime, aniComp.aniSlots[usedIdx].nTimeScale);

			if (slotIdx == -1)
			{
				aniComp.aniSlots[1].nTimeScale = aniComp.aniSlots[0].nTimeScale;
				aniComp.aniSlots[1].transitionTime = aniComp.aniSlots[0].transitionTime;
			}
		}

		ImGui::PopItemWidth();
	}
	ImGui::End();
#endif

	// Imgui bloom
	static float bloomBufferLerpVal = 0.04f;
	static int numMips = 7;
	ImGui::Begin("Bloom settings");
	ImGui::SliderFloat("Bloom lerp alpha", &bloomBufferLerpVal, 0.0f, 1.0f);
	ImGui::SliderInt("Bloom mip levels", &numMips, 0, 10);
	ImGui::End();
	Scene::setBloomBufferLerpAlpha(bloomBufferLerpVal);
	Scene::setBloomNumMipLevels(numMips);

	ImGui::Begin("Bloom");
	ImGui::SliderFloat3("Bloom color", &this->bloomColor[0], 0.0f, 100.0f);
	ImGui::SliderFloat("Bloom strength", &this->bloomStrength, 0.0f, 100.0f);
	ImGui::End();

	for (uint32_t i = 2; i < 4; ++i)
	{
		MeshComponent& meshC =
			this->getComponent<MeshComponent>(this->aniIDs[i]);

		Material* mat = nullptr;

		if (i == 2)
		{
			mat = &meshC.overrideMaterials[0];

		}
		else if (i == 3)
		{
			mat = &this->getResourceManager()->getMaterial(meshC, 0);
		}

		mat->emissionColor = this->bloomColor;
		mat->emissionIntensity = this->bloomStrength;
	}

	// Imgui directional light
	DirectionalLight& dirLight = 
		this->getComponent<DirectionalLight>(this->directionalLightEntity);
	ImGui::Begin("Cascades");
	ImGui::SliderFloat("Size 0", &dirLight.cascadeSizes[0], 0.0f, 1000.0f);
	ImGui::SliderFloat("Size 1", &dirLight.cascadeSizes[1], 0.0f, 1000.0f);
	ImGui::SliderFloat("Size 2", &dirLight.cascadeSizes[2], 0.0f, 1000.0f);
	ImGui::SliderFloat("Depth", &dirLight.cascadeDepthScale, 1.0f, 50.0f);
	ImGui::Checkbox("Visualize cascades", &dirLight.cascadeVisualization);
	ImGui::End();

	// Rotate testEntity
	/*this->getComponent<Transform>(this->testEntity).rotation.x +=
		180.0f * Time::getDT(); */

	if (Input::isKeyPressed(Keys::R))
	{
		Material& mat = this->getResourceManager()->getMaterial(
			this->getResourceManager()->getMesh(
				this->getComponent<MeshComponent>(this->testEntity).meshID
			).getSubmesh(0).materialIndex
		);

		mat.specularTextureIndex = this->getResourceManager()->addTexture("vengine_assets/textures/FullSpecular.png");
		mat.descriptorIndex = ~0u;
	}

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
	Scene::getUIRenderer()->renderString(
		"fps: " + std::to_string(1.0 / Time::getDT()),
		glm::vec3(0.0f),
		glm::vec2(50.0f),
		0.0f,
		StringAlignment::CENTER,
		glm::vec4(1.0f, 0.0f, 0.0f, 0.5f)
	);

	// Debug rendering

	// Particle system
	for (size_t i = 0; i < this->particleSystemEntities.size(); ++i)
	{
		Scene::getDebugRenderer()->renderParticleSystemCone(
			this->particleSystemEntities[i]
		);
	}

	// Skeleton
	Scene::getDebugRenderer()->renderSkeleton(
		this->aniIDs[2],
		glm::vec3(1.0f, 1.0f, 0.0f)
	);


	this->timer += Time::getDT();
#if 1
	if (ImGui::Begin("Sound"))
	{
		if (Input::isKeyDown(Keys::B))
			this->getAudioHandler()->releaseAudioSource(multiAudio);

		static int current = 0;
		static int bufferid = audioBuffer1;
		static float volume = 1.f;
		static float pitch = 1.f;
		AudioHandler* hAudio = getAudioHandler();

		ImGui::PushItemWidth(-120.f);
		ImGui::Text("Entities:");
		if (ImGui::BeginCombo("##Entities", ("Entity: " + std::to_string(audioSources[current])).c_str()))
		{
			for (int i = 0; i < 16; i++)
			{
				if (ImGui::Selectable(("Entity: " + std::to_string(audioSources[i])).c_str()))
				{
					current = i;
				}
			}
			ImGui::EndCombo();
		}
		ImGui::Text("Num Active: %u", hAudio->getNumActiveSources());
		ImGui::Separator();

		ImGui::RadioButton("Test Audio", &bufferid, audioBuffer1);
		ImGui::RadioButton("Ouf Sound", &bufferid, audioBuffer2);

		ImGui::DragFloat("Volume", &volume, 0.01f, 0.f, 6.f);
		ImGui::DragFloat("Pitch", &pitch, 0.01f, 0.f, 6.f);
		if (ImGui::Button("Play"))
		{
			hAudio->playSound(audioSources[current], bufferid, volume, pitch);
		}
		glm::vec3 pos1 = getComponent<Transform>(audioSources[current]).position;
		ImGui::Text("Pos: (%.1f, %.1f, %.1f)", pos1.x, pos1.y, pos1.z);

		
		ImGui::Separator();
		
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

	static bool open = true;
	ImGui::Begin("Cam", &open);
	glm::vec3 camPos = getComponent<Transform>(getMainCameraID()).position;
	ImGui::Text("Pos: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);
	ImGui::End();

	if (Input::isKeyPressed(Keys::O)) {
		this->getNetworkHandler()->createServer();
		this->getNetworkHandler()->createClient();
		this->getNetworkHandler()->connectClientToThis();
	}
	if (Input::isKeyPressed(Keys::I)) {
		sf::Packet packet;
		packet << (int)NetworkEvent::START;
		this->getNetworkHandler()->sendDataToServerTCP(packet);
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