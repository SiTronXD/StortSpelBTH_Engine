#pragma once
#include "../../application/Scene.hpp"
#include "../../ai/PathFinding.h"
#include "../NetworkEnumAndDefines.h"
#include <SFML/Network.hpp>

class Server;

class NetworkScene : public Scene
{
private:
	std::vector<sf::Packet>* serverToClientTCP;
	std::vector<sf::Packet>* serverToClientUDP;

protected:
	Server* server;

	//id, type
	std::vector<int> players;
	PathFindingManager pf;
public:
	NetworkScene();
	virtual ~NetworkScene();
	
	void setServer(Server* server);
    Server* getServer();

	//AI things
	PathFindingManager& getPathFinder();
	void addPolygon(NavMesh::Polygon& polygon);
	void addPolygon(const std::vector<float>& polygon);
	void removeAllPolygons();

	/////////////custom function for server only/////////////
	int createPlayer();
	int getPlayer(const int& whatPlayer);
	int getNearestPlayer(const int& ent);
	bool isAPlayer(const int& entity);
	const int getPlayerSize();
	void removePlayer(int playerID);

	/////////////////////////////////////////////////////////
	
	//Time::getDT() doesn't exist so must do extra here
	void removeAllEntitys();
	void updateSystems(float dt);

	virtual void init();
	virtual void update();
	virtual void update(float dt);
	virtual void onDisconnect(int index);

	void givePacketInfo(std::vector<sf::Packet>* serverToClientTCP);
	void givePacketInfoUdp(std::vector<sf::Packet>* serverToClientUdp);

	template <typename I, typename F>
	void addEvent(std::initializer_list<I> ints, std::initializer_list<F> floats)
	{
		//always 0 in "this->serverToClient[0][i]"
		//beacuse it points to the first object in array that doesn't exist
		//so it points at start
		for (int i = 0; i < serverToClientTCP->size(); i++)
		{
			for (auto iel : ints)
			{
				(*this->serverToClientTCP)[i] << (I)iel;
			}
			for (auto fel : floats)
			{
				(*this->serverToClientTCP)[i] << (F)fel;
			}
		}
	}
	template <typename I>
	void addEvent(std::initializer_list<I> ints)
	{
		//always 0 in "this->serverToClient[0][i]"
		//beacuse it points to the first object in array that doesn't exist
		//so it points at start
		for (int i = 0; i < serverToClientTCP->size(); i++)
		{
			for (auto el : ints)
			{
				(*this->serverToClientTCP)[i] << el;
			}
		}
	}
	template <typename I, typename F>
	void addEvent(std::vector<I> ints, std::vector<F> floats)
	{
		//always 0 in "this->serverToClient[0][i]"
		//beacuse it points to the first object in array that doesn't exist
		//so it points at start
		for (int i = 0; i < serverToClientTCP->size(); i++)
		{
			for (auto el : ints)
			{
				(*this->serverToClientTCP)[i] << el;
			}
			for (auto el : floats)
			{
				(*this->serverToClientTCP)[i] << el;
			}
		}
	}
	//UDP
    template <typename I, typename F>
    void addEventUdp(std::initializer_list<I> ints, std::initializer_list<F> floats)
    {
        //always 0 in "this->serverToClient[0][i]"
        //beacuse it points to the first object in array that doesn't exist
        //so it points at start
        for (int i = 0; i < serverToClientUDP->size(); i++)
        {
            for (auto iel : ints)
            {
                (*this->serverToClientUDP)[i] << (I)iel;
            }
            for (auto fel : floats)
            {
                (*this->serverToClientUDP)[i] << (F)fel;
            }
        }
    }
    template <typename I>
    void addEventUdp(std::initializer_list<I> ints)
    {
        //always 0 in "this->serverToClient[0][i]"
        //beacuse it points to the first object in array that doesn't exist
        //so it points at start
        for (int i = 0; i < serverToClientUDP->size(); i++)
        {
            for (auto el : ints)
            {
                (*this->serverToClientUDP)[i] << el;
            }
        }
    }
    template <typename I, typename F>
    void addEventUdp(std::vector<I> ints, std::vector<F> floats)
    {
        //always 0 in "this->serverToClient[0][i]"
        //beacuse it points to the first object in array that doesn't exist
        //so it points at start
        for (int i = 0; i < serverToClientUDP->size(); i++)
        {
            for (auto el : ints)
            {
                (*this->serverToClientUDP)[i] << el;
            }
            for (auto el : floats)
            {
                (*this->serverToClientUDP)[i] << el;
            }
        }
    }
};