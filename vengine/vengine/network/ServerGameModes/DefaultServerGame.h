#pragma once
#include "../ServerEngine/NetworkSceneHandler.h"

class DefaultServerGame : public NetworkScene
{
private:
    //AIHandler aiHandler;
public:
    DefaultServerGame();
	void start();
    void update(float dt);
};