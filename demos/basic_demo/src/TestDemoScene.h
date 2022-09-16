#pragma once

#include "vengine.h"

#include "Room.hpp"

class TestDemoScene : public Scene
{
private:
	int testEntity;
	int testEntity2;

	std::vector<int> rooms;
public:
	TestDemoScene();
	virtual ~TestDemoScene();

	// Inherited via Scene
	virtual void init() override;
	virtual void update() override;
};

