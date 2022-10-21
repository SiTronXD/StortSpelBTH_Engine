#include "TheServerGame.h"

TheServerGame::TheServerGame()
{

}

void TheServerGame::init() 
{
    int e = this->scene->createEntity();
    this->scene->setComponent<Transform>(e);
    this->scene->setScriptComponent(e, "../../vengine/vengine_assets/scripts/ServerLuaTest.lua");
    std::cout << "made script" << std::endl;
}

void TheServerGame::update(float dt)
{
    static bool wentIn = false;
    if (this->scene->getComponent<Transform>(this->scene->getPlayer(0)).position.z > 0 && !wentIn)
    {
        std::cout << "step over 1" << std::endl;
        wentIn = true;
        this->scene->createEnemy(1);
    }
    for (int i = 0; i < this->scene->getEnemySize(); i++)
    {
        this->scene->getComponent<Transform>(this->scene->getEnemies(i)).rotation += dt * 50 * (i + 1);
    }
}