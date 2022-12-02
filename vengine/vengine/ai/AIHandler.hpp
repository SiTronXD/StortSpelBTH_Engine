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
    

    void updateEntityFSMs()
	{
        auto& reg = this->sh->getScene()->getSceneReg();

		reg.view<FSMAgentComponent>(entt::exclude<Inactive>).each(
		    [&](const auto& entity, FSMAgentComponent& t)
		    { t.execute(static_cast<int>(entity)); }
		);
	}

public: 
    AIHandler() = default;
	std::unordered_map<std::string, FSM*> FSMs;
	std::unordered_map<FSM*, std::function<void(FSM* fsm, uint32_t)>> FSMimguiLambdas;
    std::unordered_map<FSM*, std::vector<uint32_t>> FSMsEntities;

    float getDeltaTime(){return this->dt;}

    void addFSM(FSM* fsm, const std::string& name) { 
		
        auto foundFSM = this->FSMs.find(name);
        if(foundFSM == this->FSMs.end())
        {
            fsm->init(this->sh, &this->eventSystem, name);
            this->FSMs.insert({fsm->getName(), fsm});
        }
    }

    void addImguiToFSM(const std::string& name, std::function<void(FSM* fsm, uint32_t)> imguiLambda) 
    {        
        this->FSMimguiLambdas[this->FSMs[name]] = imguiLambda;		
    }

    // Init all FSM and BT related things
    void init(SceneHandler *sh)
    {
        this->sh = sh; 
		this->eventSystem.setSceneHandler(this->sh);
        this->eventSystem.clean();
        this->currentScene = this->sh->getScene();
        this->switchedScene = true;

        this->FSMimguiLambdas.clear();
        this->FSMsEntities.clear();
    }

	void clean()
	{
		for (auto p : this->FSMs)
		{
			p.second->clean();
		}
	}

    void createAIEntity(uint32_t entityID, FSM* fsm)
	{
		// Register entityID to a specific FSM (MovementFSM in this test)
		this->sh->getScene()->setComponent<FSMAgentComponent>(entityID, fsm);

        // Add Entity to IMGUI lambdas vector 
        this->FSMsEntities[fsm].push_back(entityID);

		// Add required FSM and BT Components to entityID
        fsm->registerEntity(entityID);

		// Register this entity to all entity evenets of the FSM
		this->eventSystem.registerEntityToEvent(entityID, fsm);
	}

    void createAIEntity(uint32_t entityID, const std::string& fsmName)
	{
		createAIEntity(entityID, this->FSMs[fsmName]);
	}

    void resetEventSystemLastReturn()
    {
        this->eventSystem.resetEntityLastReturn();
    }

    void drawImgui();
    Scene* currentScene = nullptr; //TODO make const...
    bool disableAI = false; 
    
    void update(float dt){
        this->dt = dt;
        if(currentScene != this->sh->getScene())
        {
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