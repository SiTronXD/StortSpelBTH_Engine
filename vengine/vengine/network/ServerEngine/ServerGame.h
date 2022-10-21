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

//change naming to serverMode?
class ServerGameMode
{
protected:
    //GameDATA
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
	virtual void               init() = 0;
    const int                  getSeed();
};