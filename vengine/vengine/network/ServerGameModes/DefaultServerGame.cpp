#include "DefaultServerGame.h"

DefaultServerGame::DefaultServerGame() {

}

void DefaultServerGame::init() {

}

void DefaultServerGame::update(float dt)
{
    //static bool wentIn = false;
    //if (players[0].position.z > 0 && !wentIn)
    //    {
	//	    std::cout << "step over 1" << std::endl;
    //        wentIn = true;
    //        serverEntities.push_back(ServerEntity { glm::vec3(0, 0, 70), glm::vec3(0, 0, 0), 1 });
    //        addEvent({ (int)GameEvents::SpawnEnemy, 1 }, { 0.f, 0.f, 70.f });
	//	    int e = this->scene->createEntity();
	//	    this->scene->setComponent<Transform>(e);
	//	    this->scene->setScriptComponent(e, "network/ServerLuaTest.lua");
	//	    std::cout << "made script" << std::endl;
    //    }
    //for (int i = 0; i < serverEntities.size(); i++)
    //    {
    //        serverEntities[i].rotation.x += dt * 5 * (i + 1);
    //        if (serverEntities[i].rotation.x > 360)
    //            {
    //                serverEntities[i].rotation.x = 0;
    //            }
    //    }
}