#pragma once
#include "Room.hpp"

std::string typeToString(ROOM_TYPE type)
{
	std::string ret;
	switch (type)
	{
	case START:
		ret = "Start";
		break;
	case NORMAL:
		ret = "Normal";
		break;
	case HARD:
		ret = "Hard";
		break;
	case BOSS:
		ret = "Boss";
		break;
	case EXIT:
		ret = "Exit";
		break;
	default:
		ret = "?";
		break;
	}
	return ret;
}

void initRooms(Scene& scene, std::vector<int>& rooms)
{
	int numRooms = setUpRooms(scene, rooms);
	int numBranches = rand() % (numRooms - 2) * 2 + 1;
	for (int i = 0; i < numBranches; i++) {
		setRandomBranch(scene, rooms);
	}
	setBoss(scene, rooms, numRooms);
	setExit(scene, rooms);

}

int setUpRooms(Scene& scene, std::vector<int>& rooms)
{
	const float MIN_WIDTH = 50.0f;
	const float MAX_WIDTH = 200.0f;
	const float MIN_HEIGHT = 50.0f;
	const float MAX_HEIGHT = 200.0f;
	const float MIN_DEPTH = 50.0f;
	const float MAX_DEPTH = 200.0f;


	const float MIN_X_POS_SPREAD = -50.0f;
	const float MAX_X_POS_SPREAD = 50.0f;
	const float MIN_Y_POS_SPREAD = 0.0f;
	const float MAX_Y_POS_SPREAD = 0.0f;
	const float MIN_Z_POS_SPREAD = 0.0f;
	const float MAX_Z_POS_SPREAD = 20.0f;


	int numRooms = rand() % 15 + 4;

	
	glm::vec2 offset = glm::vec2(0.0f, 0.0f);

	for (int i = 0; i < numRooms; i++)
	{
		// Create entity (already has transform)
		rooms.push_back(scene.createEntity());
		scene.setComponent<Room>(rooms[i]);

		Room& curRoom = scene.getComponent<Room>(rooms[i]);
		Transform& curTransform = scene.getComponent<Transform>(rooms[i]);
		glm::vec3& curPos = curTransform.position;
		glm::vec2& dimensions = curRoom.dimensions;

		//First room is alwas the start room
		if (i == 0)
		{
			curRoom.type = ROOM_TYPE::START;
		}
		else
		{

			//one in five to become a hard room
			if (rand() % 5 == 0)
			{
				curRoom.type = ROOM_TYPE::HARD;
			}
			else
			{
				curRoom.type = ROOM_TYPE::NORMAL;
			}

			curRoom.dimensions = getRandomVec2(MIN_WIDTH, MAX_WIDTH, MIN_HEIGHT, MAX_HEIGHT);
			offset.x = dimensions.x / 2.0f + scene.getComponent<Room>(rooms[i - 1]).dimensions.x / 2.0f;
			offset.y = dimensions.y / 2.0f + scene.getComponent<Room>(rooms[i - 1]).dimensions.y / 2.0f;
		}

		float minX = curPos.x - offset.x + MIN_X_POS_SPREAD;
		float maxX = curPos.x + offset.x + MAX_X_POS_SPREAD;
		float minY = curPos.y + offset.y + MIN_Y_POS_SPREAD;
		float maxY = curPos.y + offset.y + MAX_Y_POS_SPREAD;

		curPos = getRandomVec3(minX, maxX, minY, maxY, 0.0f, 0.0f);

		if (i > 0)
		{
			scene.getComponent<Room>(rooms[i - 1]).up = i;
			curRoom.down = i - 1;
		}

	}

	return numRooms;
}

void setRandomBranch(Scene& scene, std::vector<int>& rooms)
{
	int branchSize = rand() % 3 + 1;
	bool foundSpot = false;
	int numMainRooms = (int)rooms.size();
	int spot = rand() % (numMainRooms - 1);
	Room& roomRef = scene.getComponent<Room>(rooms[spot]);

	if (roomRef.left != -1 && roomRef.right != -1)
	{
		//Keep looking for a spot to place branch
		while (!foundSpot)
		{
			roomRef = scene.getComponent<Room>(rooms[++spot]);
			if (spot >= numMainRooms)
			{
				spot = 0;
			}
			if (roomRef.left == -1 || roomRef.right == -1)
			{
				foundSpot = true;
			}
		}
	}
	roomRef = scene.getComponent<Room>(rooms[spot]);
	//Found spot
	if (roomRef.left == -1 && roomRef.right == -1)
	{
		if (rand() % 2 == 0)
		{
			setBranch(scene, rooms, spot, true, branchSize);
		}
		else
		{
			setBranch(scene, rooms, spot, false, branchSize);
		}
	}
	else if (roomRef.left == -1)
	{
		setBranch(scene, rooms, spot, true, branchSize);
	}
	else
	{
		setBranch(scene, rooms, spot, false, branchSize);
	}


}

void setBranch(Scene& scene, std::vector<int>& rooms, int index, bool left, int size)
{
	const float MAX_WIDTH = 200.0f;
	const float MAX_HEIGHT = 200.0f;
	const float MIN_WIDTH = 50.0f;
	const float MIN_HEIGHT = 50.0f;

	const float MIN_X_POS_SPREAD = -50.0f;
	const float MAX_X_POS_SPREAD = 50.0f;
	const float MIN_Y_POS_SPREAD = 0;
	const float MAX_Y_POS_SPREAD = 20.0f;

	ROOM_TYPE roomType = ROOM_TYPE::NORMAL;

	glm::vec2 offset = glm::vec2(0.0f, 0.0f);

	glm::vec3& position = scene.getComponent<Transform>(rooms[index]).position;
	glm::vec2 dimensions = getRandomVec2(MIN_WIDTH, MAX_WIDTH, MIN_HEIGHT, MAX_HEIGHT);

	if (rand() % 5 == 0) {
		roomType = ROOM_TYPE::HARD;
	}
	else {
		roomType = ROOM_TYPE::NORMAL;
	}

	if (left) {
		for (int i = 0; i < size; i++)
		{
			offset.x = dimensions.x / 2.0f + scene.getComponent<Room>(rooms[index]).dimensions.x / 2.0f;
			offset.y = dimensions.y / 2.0f + scene.getComponent<Room>(rooms[index]).dimensions.y / 2.0f;

			float minX = position.x - offset.x - 200.0f;
			float maxX = position.x + offset.x - 100.0f;
			float minY = position.y + offset.y - 10.0f;
			float maxY = position.y + offset.y + 10.0f;

			position = getRandomVec3(minX, maxX, minY, maxY, 0.0f, 1.0f);

			rooms.push_back(scene.createEntity());
			scene.setComponent<Room>(rooms[rooms.size() - 1]);
			Room& roomRef = scene.getComponent<Room>(rooms[rooms.size() - 1]);
			glm::vec3 posRef = scene.getComponent<Transform>(rooms[rooms.size() - 1]).position;
			roomRef.branch = true;
			roomRef.type = roomType;
			roomRef.dimensions = dimensions;
			posRef = position;

			if (i == 0)
			{
				scene.getComponent<Room>(rooms[index]).left = (int)rooms.size() - 1;
				scene.getComponent<Room>(rooms[(int)rooms.size() - 1]).right = index;
			}
			else
			{
				scene.getComponent<Room>(rooms[(int)rooms.size() - 2]).left = (int)rooms.size() - 1;
				scene.getComponent<Room>(rooms[(int)rooms.size() - 1]).right = (int)rooms.size() - 2;
			}

		}
	}
	else {

		for (int i = 0; i < size; i++)
		{
			offset.x = dimensions.x / 2.0f + scene.getComponent<Room>(rooms[index]).dimensions.x / 2.0f;
			offset.y = dimensions.y / 2.0f + scene.getComponent<Room>(rooms[index]).dimensions.y / 2.0f;

			float minX = position.x - offset.x + 100.0f;
			float maxX = position.x + offset.x + 200.0f;
			float minY = position.y + offset.y - 10.0f;
			float maxY = position.y + offset.y + 10.0f;

			position = getRandomVec3(minX, maxX, minY, maxY, 0.0f, 1.0f);

			rooms.push_back(scene.createEntity());
			scene.setComponent<Room>(rooms[rooms.size() - 1]);
			Room& roomRef = scene.getComponent<Room>(rooms[rooms.size() - 1]);
			glm::vec3 posRef = scene.getComponent<Transform>(rooms[rooms.size() - 1]).position;
			roomRef.branch = true;
			roomRef.type = roomType;
			roomRef.dimensions = dimensions;
			posRef = position;

			if (i == 0)
			{
				scene.getComponent<Room>(rooms[index]).right = (int)rooms.size() - 1;
				scene.getComponent<Room>(rooms[(int)rooms.size() - 1]).left = index;
			}
			else
			{
				scene.getComponent<Room>(rooms[(int)rooms.size() - 2]).right = (int)rooms.size() - 1;
				scene.getComponent<Room>(rooms[(int)rooms.size() - 1]).left = (int)rooms.size() - 2;
			}

		}
	}
}

void setBoss(Scene& scene, std::vector<int>& rooms, int numRooms)
{
	int left = -1;
	int bossIndex = rand() % (numRooms / 2) + numRooms / 2;
	while (scene.getComponent<Room>(rooms[bossIndex]).left != -1 && scene.getComponent<Room>(rooms[bossIndex]).right != -1) {
		if (++bossIndex > numRooms) {
			bossIndex = 1;
		}
	}
	if (scene.getComponent<Room>(rooms[bossIndex]).left == -1 && scene.getComponent<Room>(rooms[bossIndex]).right == -1) {
		left = rand() % 2;
	}
	else if (scene.getComponent<Room>(rooms[bossIndex]).left == -1) {
		left = 1;
	}
	else {
		left = 0;
	}
	setBranch(scene, rooms, bossIndex, left, rand() % 3 + 1);
	scene.getComponent<Room>(rooms[rooms.size() - 1]).type = ROOM_TYPE::BOSS;
}

void setExit(Scene& scene, std::vector<int>& rooms)
{
	int exitIndex = rand() % rooms.size() + 1;
	while (scene.getComponent<Room>(rooms[exitIndex]).type == ROOM_TYPE::BOSS || scene.getComponent<Room>(rooms[exitIndex]).type == ROOM_TYPE::START) {
		if (++exitIndex >= rooms.size()) {
			exitIndex = 1;
		}
	}
	while (scene.getComponent<Room>(rooms[exitIndex]).left != -1 && scene.getComponent<Room>(rooms[scene.getComponent<Room>(rooms[exitIndex]).left]).type == ROOM_TYPE::BOSS) {
		if (++exitIndex >= rooms.size()) {
			exitIndex = 1;
		}
	}
	while (scene.getComponent<Room>(rooms[exitIndex]).right != -1 && scene.getComponent<Room>(rooms[scene.getComponent<Room>(rooms[exitIndex]).right]).type == ROOM_TYPE::BOSS) {
		if (++exitIndex >= rooms.size()) {
			exitIndex = 1;
		}
	}
	scene.getComponent<Room>(rooms[exitIndex]).type = ROOM_TYPE::EXIT;
}


void traverseRooms(Scene& scene, std::vector<int>& rooms)
{
	std::string input;

	int id = 0;
	Room* curRoom = &scene.getComponent<Room>(rooms[id]);
	bool exit = false;
	bool foundBoss = false;
	while (!exit)
	{
		system("cls");
		std::cout << "Current Room: " << typeToString(curRoom->type) << " ID: " << id << std::endl << "Choises: " << std::endl;
		if (curRoom->down != -1) {
			std::cout << "down\n";
		}
		if (curRoom->left != -1) {
			std::cout << "left\n";
		}
		if (curRoom->right != -1) {
			std::cout << "right\n";
		}
		if (curRoom->up != -1) {
			std::cout << "up\n";
		}
		std::cout << "exit\n";
		std::cout << "input room: ";
		std::cin >> input;

		if (input == "down" && curRoom->down != -1) {
			id = curRoom->down;
			curRoom = &scene.getComponent<Room>(rooms[curRoom->down]);
		}
		else if (input == "left" && curRoom->left != -1) {
			id = curRoom->left;
			curRoom = &scene.getComponent<Room>(rooms[curRoom->left]);
		}
		else if (input == "right" && curRoom->right != -1) {
			id = curRoom->right;
			curRoom = &scene.getComponent<Room>(rooms[curRoom->right]);
		}
		else if (input == "up" && curRoom->up != -1) {
			id = curRoom->up;
			curRoom = &scene.getComponent<Room>(rooms[curRoom->up]);
		}
		else if (input == "exit" || input == "end") {
			break;
		}

		if ((curRoom->type == ROOM_TYPE::BOSS) && !foundBoss) {
			foundBoss = true;
		}

		if ((curRoom->type == ROOM_TYPE::EXIT) && foundBoss) {
			system("cls");
			std::cout << "You found the exit!\nDo you want to exit? Y/n" << std::endl;
			std::cin >> input;
			if (input == "Y") {
				exit = true;
			}
		}
	}
}
