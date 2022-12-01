#pragma once
#include "../../application/SceneHandler.hpp"
#include "NetworkScene.h"
#include "../../dev/Fail.h"
#include <functional>
#include <queue>

class NetworkSceneHandler : public SceneHandler
{
	friend class Server;
  private:
	std::vector<sf::Packet>* serverToClientPacketTcp;
	std::vector<sf::Packet>* serverToClientPacketUdp;
	sf::Packet callsFromClient;

  public:
	std::function<void()> clientGetFunc;
	std::function<void()> clientStopFunc;
	NetworkSceneHandler();
	void update(float dt);
	void givePacketInfo(std::vector<sf::Packet>& serverToClientTCP);
	void givePacketInfoUdp(std::vector<sf::Packet>& serverToClientUdp);
	void updateToNextScene();
	void setScene(Scene* scene, std::string path = "");

	sf::Packet& getCallFromClient();

	NetworkScene* getScene() const;

	void setGetClientFunction(std::function<void()> f) { this->clientGetFunc = f; };
	void setStopClientFunction(std::function<void()> f) { this->clientStopFunc = f;  };

	//normal update doesn't work in network, remove it from networkSceneHandler
	template <typename T = bool>
	void update()
	{
		static_assert(fail<T>::value, "Do not use this function! Use update(float dt)");
	}

	template <typename I, typename F>
	void sendCallFromClient(std::initializer_list<I> ints, std::initializer_list<F> floats)
	{
			for (auto el : ints)
			{
				this->callsFromClient << el;
			}
			for (auto el : floats)
			{
				this->callsFromClient << el;
			}
	}
	template <typename I>
	void sendCallFromClient(std::initializer_list<I> ints)
	{
			for (auto el : ints)
			{
				this->callsFromClient << el;
			}
	}
};
