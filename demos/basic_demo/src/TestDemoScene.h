#pragma once

#include "vengine.h"

class TestDemoScene : public Scene
{
private:
	int testEntity;
	int testEntity2;

	uint32_t uiTextureIndex0;
	uint32_t uiTextureIndex1;

	float timer;

public:
	TestDemoScene();
	virtual ~TestDemoScene();

	// Inherited via Scene
	virtual void init() override;
	virtual void update() override;
};

