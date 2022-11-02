#pragma once
#include "../../application/SceneHandler.hpp"
#include "NetworkScene.h"
#include "../../dev/Fail.h"

class NetworkSceneHandler : public SceneHandler
{
  private:
	std::vector<sf::Packet>* serverToClientPacketTcp;
  public:
	NetworkSceneHandler();
	void update(float dt);
	void givePacketInfo(std::vector<sf::Packet>& serverToClient);
	void updateToNextScene();
	void setScene(Scene* scene, std::string path = "");

	NetworkScene* getScene() const;

	//normal update doesn't work in network, remove it from networkSceneHandler
	template <typename T = bool>
	void update()
	{
		static_assert(fail<T>::value, "Do not use this function! Use update(float dt)");
	}
};
