#pragma once

#include "vengine.h"
#define AUDIO 1
class TestDemoScene : public Scene
{
private:
	Entity camEntity;
	Entity testEntity;
	Entity testEntity2;
	Entity multiAnimation;
	Entity directionalLightEntity;
	Entity particleSystemEntity;
	uint32_t uiTextureIndex0;
	uint32_t uiTextureIndex1;
	uint32_t fontTextureIndex;

	float timer;

	glm::vec3 bloomColor = glm::vec3(1.0f, 0.0f, 1.0f);
	float bloomStrength = 1.0f;

	Entity aniIDs[4];
	bool aniActive[4];

	Entity audioSource1;
	Entity audioSource2;
	float volume1;
	float volume2;
	float master;
	float music;
	Entity multiAni2;
    int floor;
	bool rotDir = false;
public:
	TestDemoScene();
	virtual ~TestDemoScene();

	//  Inherited via Scene
	virtual void init() override;
	virtual void start() override;
	virtual void update() override;

	virtual void onCollisionEnter(Entity e1, Entity e2) override;
	virtual void onCollisionStay(Entity e1, Entity e2) override;
	virtual void onCollisionExit(Entity e1, Entity e2) override;
	virtual void onTriggerEnter(Entity e1, Entity e2) override;
	virtual void onTriggerStay(Entity e1, Entity e2) override;
	virtual void onTriggerExit(Entity e1, Entity e2) override;
};

