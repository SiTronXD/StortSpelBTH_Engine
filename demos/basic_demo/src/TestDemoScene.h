#pragma once

#include "vengine.h"

#include "Room.hpp"

class TestDemoScene : public Scene
{
private:
	int testEntity;
	int testEntity2;

	//Room test variables
	int boss;
	std::vector<int> rooms;
	int doors[4];
	bool foundBoss;
	int bossHealth;
	int roomID;
public:
	TestDemoScene();
	virtual ~TestDemoScene();

	// Inherited via Scene
	virtual void init() override;
	virtual void update() override;

private:
	//Room stuff
	void setRoomVar();
	void setUpRooms();
	void runRoomIteration();

};

