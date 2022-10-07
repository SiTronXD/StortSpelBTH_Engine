#pragma once
#include <vector>
#include "glm/vec3.hpp"
#include "SFML/Network.hpp"
#include "NetworkEnumAndDefines.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

struct ServerEntity
{
    glm::vec3 position;
    glm::vec3 rotation;
    int       type;
};

//change naming to serverMode?
class ServerGame
{
protected:
    //GameDATA
    std::vector<ServerEntity> serverEntities;
    std::vector<ServerEntity> players;
    int                       seed;

public:
    std::string n;

    ServerGame();
    virtual ~ServerGame();

    virtual void               update(float dt) = 0;
    void                       createPlayer();
    std::vector<ServerEntity>& getServerEntities();
    std::vector<ServerEntity>& getServerPlayers();
    void                       GivePacketInfo(std::vector<sf::Packet>& serverToClient);
    const int                  getSeed();

protected:
    std::vector<sf::Packet>* serverToClient;

    template <typename I, typename F>
    void addEvent(std::initializer_list<I> ints, std::initializer_list<F> floats)
    {
        //always 0 in "this->serverToClient[0][i]"
        //beacuse it points to the first object in array that doesn't exist
        //so it points at start
        for (int i = 0; i < serverToClient->size(); i++)
            {
                for (auto el : ints)
                    {
                        this->serverToClient[0][i] << el;
                    }
                for (auto el : floats)
                    {
                        this->serverToClient[0][i] << el;
                    }
            }
    }
};