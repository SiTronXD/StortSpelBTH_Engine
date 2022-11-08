#pragma once
#include "../ServerEngine/NetworkSceneHandler.h"

class NetworkLobbyScene : public NetworkScene
{
  public:
	NetworkLobbyScene();
	void init();
	void update(float dt);
};