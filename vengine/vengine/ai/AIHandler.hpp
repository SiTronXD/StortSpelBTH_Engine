#pragma once
#include "FSM.hpp"
#include "../application/SceneHandler.hpp"
#include "../components/FSMAgentComponent.hpp"
#include "PathFinding.h"
#include <cstdint>

class AIHandler
{
private: 
    bool switchedScene = true;
    SceneHandler *sh = nullptr; 
    EventSystem eventSystem;
    float dt;

    void updateEntityFSMs();

  public: 
    AIHandler() = default;
	std::unordered_map<std::string, FSM*> FSMs;
	std::unordered_map<FSM*, std::function<void(FSM* fsm, uint32_t)>> FSMimguiLambdas;
    std::unordered_map<FSM*, std::vector<uint32_t>> FSMsEntities;

    inline float getDeltaTime(){return this->dt;}

    void addImguiToFSM(const std::string& name, std::function<void(FSM* fsm, uint32_t)> imguiLambda);

    // Init all FSM and BT related things
    void init(SceneHandler* sh);

    void clean();

    void createAIEntity(uint32_t entityID, FSM* fsm);
    void createAIEntity(uint32_t entityID, const std::string& fsmName);

    void resetEventSystemLastReturn();

    void drawImgui();
    Scene* currentScene = nullptr; //TODO make const...
    bool disableAI = false;

    void update(float dt);
};

            FSMimguiLambdas.clear();
            FSMsEntities.clear();
            currentScene = this->sh->getScene();
        }

        eventSystem.update();
        if(!disableAI)
        {
            updateEntityFSMs();
        }
        drawImgui();
        switchedScene = false; 
    }

};