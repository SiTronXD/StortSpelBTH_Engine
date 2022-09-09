#pragma once

#include "Scene.h"

class TestScene2 : public Scene
{
private:
	int testEntity2;

public:
	TestScene2();
	virtual ~TestScene2();

	// Inherited via Scene
	virtual void init() override;
	virtual void update() override;
};
