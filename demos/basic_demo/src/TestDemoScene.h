#pragma once

#include "vengine.h"

class TestDemoScene : public Scene
{
private:
	int stone;
	int Player;
	//other players are in networkHandler
public:
	TestDemoScene();
	virtual ~TestDemoScene();

	// Inherited via Scene
	virtual void init() override;
	virtual void update() override;
};

