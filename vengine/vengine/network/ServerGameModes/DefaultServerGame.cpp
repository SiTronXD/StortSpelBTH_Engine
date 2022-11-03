#include "DefaultServerGame.h"

DefaultServerGame::DefaultServerGame() {

}

void DefaultServerGame::init() 
{
    //TODO: After intern Test, make use of DefaultServerGame
    //this->aiHandler.init(&this->sceneHandler);
}

void DefaultServerGame::update(float dt)
{
	static bool wentIn = false;
	if (this->getComponent<Transform>(this->getPlayer(0)).position.z > 0 && !wentIn)
	{
		std::cout << "step over 1" << std::endl;
		wentIn = true;
		this->createEnemy(1);
	}
	for (int i = 0; i < this->getEnemySize(); i++)
	{
		this->getComponent<Transform>(this->getEnemies(i)).rotation += dt * 50 * (i + 1);
	}
    getPlayer(0);
    //TODO: After intern Test, make use of DefaultServerGame
    //this->aiHandler.update();
    //pf.atPoint(const glm::vec3 &from, const glm::vec3 &to) // Checks if Entity is close enough to specific position
    //pf.getDirTo(glm::vec3 &from, glm::vec3 &to)              // Returns a normalized Vector from "from" to "to"; That points in the next position player has to go to to reach position "to" 
    
}