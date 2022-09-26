#pragma once

#include <iostream>
#include "glm/glm.hpp"
#include "vengine.h"

#include "Randomizer.hpp"

enum ROOM_TYPE{START_ROOM, NORMAL_ROOM, HARD_ROOM, BOSS_ROOM, EXIT_ROOM};

struct Room
{
	int up, down, left, right;
	glm::vec3 dimensions;
	ROOM_TYPE type;
	bool branch, branchEnd, shortcut;

	Room()
		:up(-1), down(-1), left(-1), right(-1),
		dimensions(glm::vec3(10.0f, 10.0f, 10.0f)),
		type(ROOM_TYPE::NORMAL_ROOM),branch(false), branchEnd(false),
		shortcut(false)
	{
	};

};

std::string typeToString(ROOM_TYPE type);

void initRooms(Scene& scene, std::vector<int>& rooms, int doors[], int roomID);
int setUpRooms(Scene& scene, std::vector<int>& rooms);
bool setRandomBranch(Scene& scene, std::vector<int>& rooms, int numRooms);
void setBranch(Scene& scene, std::vector<int>& rooms, int index, bool left, int size);
bool setBoss(Scene& scene, std::vector<int>& rooms, int numRooms);
bool setExit(Scene& scene, std::vector<int>& rooms);
bool setShortcut(Scene& scene, std::vector<int>& rooms, int numBranches, int numRooms);

int numEnds(Scene& scene, std::vector<int>& rooms);
int getEndWithRightAvaliable(Scene& scene, std::vector<int>& rooms);
int getEndWithLeftAvaliable(Scene& scene, std::vector<int>& rooms);

void traverseRoomsConsole(Scene& scene, std::vector<int>& rooms);
bool traverseRooms(Scene& scene, std::vector<int>& rooms, int doors[], int& roomID, int& boss, int& bossHealth, bool& foundBoss, float delta);


void placeDoors(Scene& scene, std::vector<int>& rooms, int doors[], int& roomID);
bool canGoForward(Scene& scene, int doors[]);
bool canGoBack(Scene& scene, int doors[]);
bool canGoLeft(Scene& scene, int doors[]);
bool canGoRight(Scene& scene, int doors[]);
void fightBoss(Scene& scene, std::vector<int>& rooms, int doors[], int& boss, int& bossHealth, int& roomID, bool& foundBoss);
void printDoorOptions(Scene& scene, std::vector<int>& rooms, int& roomID);