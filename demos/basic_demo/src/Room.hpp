#pragma once

#include <iostream>
#include "glm/glm.hpp"
#include "vengine.h"

#include "Randomizer.hpp"

enum ROOM_TYPE{START, NORMAL, HARD, BOSS, EXIT};

struct Room
{
	int up, down, left, right;
	glm::vec3 dimensions;
	ROOM_TYPE type;
	bool branch;

	Room()
		:up(-1), down(-1), left(-1), right(-1),
		dimensions(glm::vec3(10.0f, 10.0f, 10.0f)),
		type(ROOM_TYPE::NORMAL),branch(false)
	{
	};

};

std::string typeToString(ROOM_TYPE type);

void initRooms(Scene& scene, std::vector<int>& rooms);
int setUpRooms(Scene& scene, std::vector<int>& rooms);
void setRandomBranch(Scene& scene, std::vector<int>& rooms, int numRooms);
void setBranch(Scene& scene, std::vector<int>& rooms, int index, bool left, int size);
void setBoss(Scene& scene, std::vector<int>& rooms, int numRooms);
void setExit(Scene& scene, std::vector<int>& rooms);

void traverseRoomsConsole(Scene& scene, std::vector<int>& rooms);