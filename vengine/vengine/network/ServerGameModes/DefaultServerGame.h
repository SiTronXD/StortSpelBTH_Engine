#pragma once

#include "../ServerEngine/ServerGame.h"
#include "../../ai/AIHandler.hpp"

class DefaultServerGame : public ServerGameMode
{
private:
    //AIHandler aiHandler;
public:
    DefaultServerGame();
    void init();
    void update(float dt);
};