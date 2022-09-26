#pragma once

#include "vengine.h"

#include "vengine/application/PhysicsEngine.h"

class TestDemoScene : public Scene
{
private:
	int testEntity;
	int testEntity2;

	PhysicsEngine physEngine;

public:
	TestDemoScene();
	virtual ~TestDemoScene();

	// Inherited via Scene
	virtual void init() override;
	virtual void update() override;
};

