#pragma once

#include "vengine.h"

#include "Room.hpp"

class TestDemoScene : public Scene
{
private:
	int testEntity;
	int testEntity2;

	std::vector<int> rooms;
	int doors[4];
	bool foundBoss;
	bool done;
	int roomID;
public:
	TestDemoScene();
	virtual ~TestDemoScene();

	// Inherited via Scene
	virtual void init() override;
	virtual void update() override;

private:
	bool traverseRooms();
	void placeDoors();
	bool canGoForward();
	bool canGoBack();
	bool canGoLeft();
	bool canGoRight();
};

