#pragma once

#include "vengine.h"

class NetworkTestScene : public Scene
{
private:
	int stone;
	int Player;
	//other players are in networkHandler
public:
	NetworkTestScene();
	virtual ~NetworkTestScene();

	// Inherited via Scene
	virtual void init() override;
	virtual void update() override;
};

