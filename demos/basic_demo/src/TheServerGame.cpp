#include "TheServerGame.h"

TheServerGame::TheServerGame()
{

}

void TheServerGame::init() 
{
    int ground = this->createEntity();
    this->getComponent<Transform>(ground).position = glm::vec3(0, -1, 0);
    this->setComponent<Collider>(ground, Collider::createBox(glm::vec3(100, 0.2, 100)));

    int e = this->createEnemy(1,"", glm::vec3(0,0,5));
    this->setComponent<Collider>(e, Collider::createBox(glm::vec3(100, 0.2, 100)));
    this->setComponent<Rigidbody>(e);
}

void TheServerGame::update(float dt)
{
    static bool wentIn = false;
    for (int i = 0; i < this->getEnemySize(); i++)
    {
        this->getComponent<Transform>(this->getEnemies(i)).rotation.y += dt * 50 * (i + 1);
    }
}