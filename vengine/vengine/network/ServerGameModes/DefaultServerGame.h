#pragma once

#include "../ServerEngine/ServerGame.h"

class DefaultServerGame : public ServerGameMode
{
private:

public:
    DefaultServerGame();
    void init();
    void update(float dt);
};