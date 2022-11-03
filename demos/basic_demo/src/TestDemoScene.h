#pragma once

#include "vengine.h"

class TestDemoScene : public Scene
{
private:
	Entity camEntity;
	Entity testEntity;
	Entity testEntity2;
	Entity multiAnimation;
	uint32_t uiTextureIndex0;
	uint32_t uiTextureIndex1;
	uint32_t fontTextureIndex;

	float timer;

	Entity aniIDs[4];
	bool aniActive[4];
  
  int floor;
	bool rotDir = false;
public:
	TestDemoScene();
	virtual ~TestDemoScene();

	//  Inherited via Scene
	virtual void init() override;
	virtual void update() override;

	virtual void onCollisionStay(Entity e1, Entity e2) override;
	virtual void onTriggerStay(Entity e1, Entity e2) override;
};

