#include "TheServerGame.h"

TheServerGame::TheServerGame()
{

}

void TheServerGame::init() 
{
    int e = this->scene->createEnemy(1, "../../vengine/vengine_assets/scripts/testScript.lua");
}

void TheServerGame::update(float dt)
{
    static bool wentIn = false;
    for (int i = 0; i < this->scene->getEnemySize(); i++)
    {
        this->scene->getComponent<Transform>(this->scene->getEnemies(i)).rotation.x += dt * 50 * (i + 1);
    }
}