#pragma once
#include "../vengine/vengine/network/ServerEngine/NetworkScene.h"

class TheServerGame : public NetworkScene {
private:

public:
	TheServerGame();
	void init();
	void update(float dt);
};