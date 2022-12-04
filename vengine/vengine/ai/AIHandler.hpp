#pragma once 
 #include "op_overload.hpp"
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

    template<class FSM_SUBCLASS>
    void addFSM(const std::string& name);
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

template<class FSM_SUBCLASS>
void AIHandler::addFSM(const std::string& name)
{

    auto foundFSM = this->FSMs.find(name);
    if (foundFSM == this->FSMs.end())
    {
        FSM* fsm = new(__FILE__, __LINE__) FSM_SUBCLASS;
        fsm->init(this->sh, &this->eventSystem, name);
        this->FSMs.insert({fsm->getName(), fsm});
    }
}