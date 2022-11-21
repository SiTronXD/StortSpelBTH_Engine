#pragma once
#include <vector>
#pragma region PORTS_AND_OTHER

#define UDP_PORT_CLIENT 55000
#define UDP_PORT_SERVER 55010
#define TCP_PORT_SERVER 55020
#define TCP_PORT_CLIENT 55030
#define SERVER_IP "192.168.1.104"
#define ServerUpdateRate 1 / 24.f
#define MAXNUMBEROFPLAYERS 4

#pragma endregion

struct TCPPacketEvent
{
	int gameEvent;
	int nrOfInts;
	int ints[3];  //max nr of ints is 3
	std::vector<float> floats;
};

enum StartingEnum
{
	WaitingForUsers,
	Start,
	Running
};

#pragma region GAMEEVENTS_NETWORK

enum GameEvents
{
	EMPTY = 0,
	//TCP To Client
	SpawnEnemy = 1,			// : Type(int), Position(float x,y,z);
	SpawnEnemies = 2,		// : Type(int), NrOfEnemies(int), Position(list/array)(float x,y,z)   (can only spawn one type of enemy in spawnEnemies)
    SpawnPerk = 3,			// : Type(int), Position(float x,y,z)
	MonsterDied = 4,        // : Monster_id(int)
	ID = 5,					// : ID(int)
	PlayerJoined,			//: name(sfml fix size of string), serverID(int)
	GAMEDATA,				//:?
	ROOM_CLEAR,             //: (nothing)
	MONSTER_HIT,			//: monsterID(int), Damage(int), PlayerID(int)   (a monster hit you)

	//TCP to Server
	HitMonster,				//: Monster_id(int), PlayerID(int), Damage(int), knockbackarr(float), Call to Scene
	CHANGESCEENE,			//:?
	POLYGON_DATA,			//: 
	REMOVE_POLYGON_DATA,    //: nothing
	WentInToNewRoom,		//:?
	HealPlayer,				//

	//TCP to Client and Server
	START,  //Call to Scene
	PlayerDied,
	DISCONNECT,
	PickUpPerk,

	//UDP
	UpdatePlayerPos,
	UpdateMonsterPos,
	UpdateParkPos,

	//Get from server
	GetPlayerNames,
	GetLevelSeed,

	//DEBUG
	A_Button_Was_Pressed_On_Server,
	A_Button_Was_Pressed_On_Client,
	Draw_Debug_Line,
	Draw_Debug_CapsuleCollider,
	Draw_Debug_BoxCollider,
	END
};
/*
/////////events tcp server -> client////////////////

SpawnEnemy			: Type(int), Position(float x,y,z);
SpawnEnemies		: Type(int), NrOfEnemies(int), Position(list/array)(float x,y,z)   (can only spawn one type of enemy in spawnEnemies)
Explosion			: Radious(float), Position(float x,y,z)
MonsterDied			: Monster_id, hpback(int player, int howmuch hp), perk/abilityType(int), multiplier, position, spawnDir, 
SendSeed			: int
PlayerDied			: PlayerID
Disconnected		: PlayerID
PlayerJoined		: name(sfml fix size of string), serverID(int)
MONSTER_HIT			: monsterID, Damage, PlayerID
Start				: (nothing)
Disconnected		: what player


/////////events tcp client -> server///////////////
HitMonster			: Monster_id, Damage(int), knockbackarr(float), 
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
