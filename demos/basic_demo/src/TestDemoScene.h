#pragma once

#include "vengine.h"

class TestDemoScene : public Scene
{
private:
	int testEntity;
	int testEntity2;
	int testEntity3;
	int testEntity4;

	int floor;
	bool rotDir = false;
public:
	TestDemoScene();
	virtual ~TestDemoScene();

	//  Inherited via Scene
	virtual void init() override;
	virtual void update() override;
};

