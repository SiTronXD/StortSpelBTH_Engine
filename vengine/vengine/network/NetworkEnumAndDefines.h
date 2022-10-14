#pragma once
#include <vector>
#pragma region PORTS_AND_OTHER

#define UDP_PORT_CLIENT  55001
#define UDP_PORT_SERVER  55002
#define TCP_PORT_SERVER  55003
#define TCP_PORT_CLIENT  55004
#define SERVER_IP        "192.168.1.104"
#define ServerUpdateRate 1 / 24.f

#pragma endregion

struct TCPPacketEvent {
	int                gameEvent;
	int                nrOfInts;
	int                ints[3]; //max nr of ints is 3
	std::vector<float> floats;
};

enum StartingEnum { WaitingForUsers, Start, Running };

#pragma region GAMEEVENTS_NETWORK

enum GameEvents {
	EMPTY		 = 0,
	//TCP
	SpawnEnemy   = 1,
	SpawnEnemies = 2,
	Explosion    = 3,
	MonsterDied  = 4,
	PlayerShoot  = 5,
	HitMonster	 = 6,
	PlayerDied	 = 7,
	ID,

	PlayerJoined,
	GAMEDATA,
	START,
	DISCONNECT,
	END,
	POLYGON_DATA,
	REMOVE_POLYGON_DATA,

	//UDP
	UpdatePlayerPos,
	UpdateMonsterPos,

	//DEBUG
	A_Button_Was_Pressed_On_Server,
	A_Button_Was_Pressed_On_Client
};

//In every package we must say how many events first
//could I have a Event::end instead?

//events tcp server -> client
//SpawnEnemy		: Type(int), Position(float x,y,z);
//SpawnEnemies		: Type(int), NrOfEnemies(int), Position(list/array)(float x,y,z)   (can only spawn one type of enemy in spawnEnemies)
//Explosion			: Radious(float), Position(float x,y,z)
//MonsterDied		: Monster_id
// SendSeed         : int
//PlayerDied		: PlayerID
//Disconnected		: PlayerID
// PlayerJoined		: name(sfml fix size of string)
// ID				: ID, nrOfPlayers connected - 1

//events tcp client -> server
//PlayerShoot		: Type(int), Direction (float x,y,z),
//HitMonster		: Monster_id, Damage
//HitMonsters		: Monster_ids(list/array), Damage
//
//PlayerDied		: int id
//Disconnected		: (nothig)
//Start				: (nothing)
//GAMEDATA			: NrOfPlayers - 1, PlayerIDs (list, not yours), seed,

//updates/events udp server -> client
//UpdatePlayerPos	: nrOfPlayers, position(list/array), rotation(list/array)
//UpdateMonsterPos  : nrOfMonsters, position(list/array), rotation(list/array)

//updates/events udp client -> server
//UpdatePlayerPos   : position, rotation (don't think I need to id with here beacuse ip fixes that)

#pragma endregion
