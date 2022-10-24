#pragma once
#include "../vengine/vengine/network/ServerEngine/ServerGame.h"

class TheServerGame : public ServerGameMode {
private:

public:
	TheServerGame();
	void init();
	void update(float dt);
};