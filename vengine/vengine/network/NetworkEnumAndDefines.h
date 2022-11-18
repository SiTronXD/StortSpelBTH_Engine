#pragma once
#include <vector>
#pragma region PORTS_AND_OTHER

#define UDP_PORT_CLIENT 55000
#define UDP_PORT_SERVER 55010
#define TCP_PORT_SERVER 55020
#define TCP_PORT_CLIENT 55030
#define SERVER_IP "192.168.1.104"
#define ServerUpdateRate 1 / 24.f

#pragma endregion

struct TCPPacketEvent 
{
	int                event;
	int                nrOfInts;
	int                ints[3]; // Max nr of ints is 3
	std::vector<float> floats;
};

enum class ServerStatus { WAITING, START, RUNNING };

enum class NetworkEvent
{
	ERR = -1,
	EMPTY,
	ECHO, // Repeat whatever is sent on server
	CLIENTJOINED,
	JUSTJOINED,
	DISCONNECT,
	GETNAMES,
	START,
	END
};

#pragma region GAMEEVENTS_NETWORK

//enum GameEvents {
//	EMPTY		 = 0,
//	//TCP To Client
//	SpawnEnemy = 1,
//	SpawnEnemies = 2,
//	MonsterDied = 3,
//	ID = 4,
//	PlayerJoined,
//	GAMEDATA,
//
//	//TCP to Server
//	HitMonster,				//Call to Scene
//	START,					//Call to Scene
//	CHANGESCEENE,
//	POLYGON_DATA,
//	REMOVE_POLYGON_DATA,
//
//	//TCP to Client and Server
//	Explosion,
//	PlayerShoot,
//	PlayerDied,
//	DISCONNECT,
//
//
//	//UDP
//	UpdatePlayerPos,
//	UpdateMonsterPos,
//
//	//Get from server
//	GetPlayerNames,
//	GetLevelSeed,
//
//
//	//DEBUG
//	A_Button_Was_Pressed_On_Server,
//	A_Button_Was_Pressed_On_Client,
//	END
//};

/*
/////////events tcp server -> client////////////////

SpawnEnemy			: Type(int), Position(float x,y,z);
SpawnEnemies		: Type(int), NrOfEnemies(int), Position(list/array)(float x,y,z)   (can only spawn one type of enemy in spawnEnemies)
Explosion			: Radious(float), Position(float x,y,z)
MonsterDied			: Monster_id
SendSeed			: int
PlayerDied			: PlayerID
Disconnected		: PlayerID
PlayerJoined		: name(sfml fix size of string), serverID(int)
MONSTER_HIT			: monsterID, Damage, PlayerID
Start				: (nothing)
Disconnected		: what player


/////////events tcp client -> server///////////////
HitMonster			: Monster_id, Damage
HitMonsters			: Monster_ids(list/array), Damage
PlayerDied			: int id
Disconnected		: (nothig)
Start				: (nothing)
GAMEDATA			: NrOfPlayers - 1, PlayerIDs (list, not yours), seed, // wrong
WentInToNewRoom		: 

///////////updates/events udp server -> client//////////////
UpdatePlayerPos		: nrOfPlayers(int), position(list/array of float x,y,z), rotation(list/array)
UpdateMonsterPos	: nrOfMonsters(int), position(list/array of float x,y,z), rotation(list/array)



///////////updates/events udp client -> server//////////////
UpdatePlayerPos		: position(float x,y,z), rotation(float x,y,z) (don't think I need to id with here beacuse ip fixes that)

GetPlayerNames		: nrOfPlayers(int) PlayerNames(string)
GetLevelSeed		: Seed(int)


Draw_Debug_Line		: position(list/array of float x,y,z), position(list/array of float x,y,z)
*/
#pragma endregion
