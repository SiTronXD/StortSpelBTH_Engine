#include "TheServerGame.h"

TheServerGame::TheServerGame()
{

}

void TheServerGame::init() 
{
    int e = createEnemy(1, "../../vengine/vengine_assets/scripts/testScript.lua");
}

void TheServerGame::update(float dt)
{
    static bool wentIn = false;
    for (int i = 0; i < this->getEnemySize(); i++)
    {
        this->getComponent<Transform>(this->getEnemies(i)).rotation.x += dt * 50 * (i + 1);
    }
}