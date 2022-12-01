#pragma once

#include "vengine.h"

class LobbyScene : public Scene
{
private:
	int floor;
	int Player;
public:
	LobbyScene();
	virtual ~LobbyScene();

	// Inherited via Scene
	virtual void init() override;
	virtual void start() override;
	virtual void update() override;
};

