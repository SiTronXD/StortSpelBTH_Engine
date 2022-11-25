#include "pch.h"
#include "DefaultServerGame.h"

DefaultServerGame::DefaultServerGame() {}

void DefaultServerGame::start()
{
	int ground = this->createEntity();
	this->getComponent<Transform>(ground).position = glm::vec3(0, -1, 0);
	this->setComponent<Collider>(ground, Collider::createBox(glm::vec3(100, 0.2, 100)));

	int e = this->createEnemy(0, "", glm::vec3(0, 4, 0));
	this->setComponent<Collider>(e, Collider::createBox(glm::vec3(1, 1, 1)));
	this->setComponent<Rigidbody>(e);
	this->getComponent<Rigidbody>(e).rotFactor = glm::vec3(0, 0, 0);
	e = this->createEnemy(1, "", glm::vec3(4, 5, 0));
	this->setComponent<Collider>(e, Collider::createBox(glm::vec3(1, 1, 1)));
}

void DefaultServerGame::update(float dt)
{
	static bool wentIn = false;
	for (int i = 0; i < this->getEnemySize(); i++)
	{
		this->getComponent<Transform>(this->getEnemies(i)).rotation.y += dt * 50 * (i + 1);
	}
}