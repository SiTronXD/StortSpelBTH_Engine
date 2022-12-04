#pragma once 
 #include "op_overload.hpp"
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