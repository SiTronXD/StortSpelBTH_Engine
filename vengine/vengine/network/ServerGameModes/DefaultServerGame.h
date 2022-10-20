#pragma once

#include "../ServerEngine/ServerGame.h"

class DefaultServerGame : public ServerGameMode
{
private:

public:
    DefaultServerGame();
    void update(float dt);
};