#pragma once

#include "../../ai/AIHandler.hpp"
#include "../ServerEngine/NetworkScene.h"

class DefaultServerGame : public NetworkScene
{
private:
    //AIHandler aiHandler;
public:
    DefaultServerGame();
    void init();
    void update(float dt);
};