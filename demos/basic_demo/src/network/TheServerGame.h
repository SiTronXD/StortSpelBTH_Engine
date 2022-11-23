#pragma once
#include "vengine/network/ServerEngine/NetworkScene.h"

class TheServerGame : public NetworkScene {
private:

public:
	TheServerGame();
	void init();
	void start();
	void update(float dt);
};