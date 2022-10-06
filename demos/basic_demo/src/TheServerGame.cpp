#include "TheServerGame.h"

TheServerGame::TheServerGame()
{
}

void TheServerGame::update(float dt)
{
    static bool wentIn = false;
    if (players[0].position.z > 50 && !wentIn)
    {
        wentIn = true;
        serverEntities.push_back(ServerEntity{ glm::vec3(0, 0, 50), glm::vec3(0, 0, 0), 1 });
        addEvent({ (int)GameEvents::SpawnEnemy, 1 }, { 0.f, 0.f, 70.f });

        serverEntities.push_back(ServerEntity{ glm::vec3(20, 0, 50), glm::vec3(0, 0, 0), 1 });
        addEvent({ (int)GameEvents::SpawnEnemy, 1 }, { 0.f, 0.f, 50.f });
    }
    for (int i = 0; i < serverEntities.size(); i++)
    {
        if (i % 2 == 0) {
            serverEntities[i].rotation.x += dt * 5 * (i + 1);
            if (serverEntities[i].rotation.x > 360)
            {
                serverEntities[i].rotation.x = 0;
            }
        }
        else {
            serverEntities[i].rotation.x -= dt * 5 * (i + 1);
            if (serverEntities[i].rotation.x < -360)
            {
                serverEntities[i].rotation.x = 0;
            }
        }
    }
}