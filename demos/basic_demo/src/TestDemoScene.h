#pragma once

#include "vengine.h"

class TestDemoScene : public Scene
{
private:
	int testEntity;
	int testEntity2;
	int testEntity3;
	int testEntity4;

public:
	TestDemoScene();
	virtual ~TestDemoScene();

	// Inherited via Scene
	virtual void init() override;
	virtual void update() override;
};

