#pragma once

#include "vengine.h"

class TestDemoScene : public Scene
{
private:
	Entity camEntity;
	Entity testEntity;
	Entity testEntity2;

	uint32_t uiTextureIndex0;
	uint32_t uiTextureIndex1;

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
};

