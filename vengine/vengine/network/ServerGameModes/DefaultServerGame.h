#pragma once
#include "../ServerEngine/NetworkScene.h"

class DefaultServerGame : public NetworkScene
{
private:
    //AIHandler aiHandler;
public:
    DefaultServerGame();
    void init();
	void start();
    void update(float dt);
};