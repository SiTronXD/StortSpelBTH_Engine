#pragma once
#include <vector>
#include "glm/vec3.hpp"
#include "SFML/Network.hpp"
#include "../NetworkEnumAndDefines.h"
#include "../../ai/PathFinding.h"
#include "NetworkScene.h"
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
class ServerGameMode
{
private:
    //do things faster and more reliaebly
    void spawnEnemy(glm::vec3 pos);



protected:
    //GameDATA
    std::vector<ServerEntity> serverEntities;
    std::vector<ServerEntity> players;
    int                       seed;

    PathFindingManager        pf;

    //handler + scene
    NetworkScene *scene;
	ServerScriptHandler* scriptHandler;

   public:
    void setScene(NetworkScene *scene);
	void setScriptHandler(ServerScriptHandler* scriptHandler);

	//AI things
    void addPolygon(NavMesh::Polygon& polygon);
	void addPolygon(const std::vector<float>& polygon);
	void removeAllPolygons();

    ServerGameMode();
    virtual ~ServerGameMode();

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