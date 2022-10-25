#pragma once
#include "FSM.hpp"
#include "../application/SceneHandler.hpp"
#include "../components/FSMAgentComponent.hpp"
#include <cstdint>

struct FSM_system : public System
{
   private:
   public:
	explicit FSM_system(){};

	bool update(entt::registry& reg, float deltaTime) override
	{
		reg.view<FSMAgentComponent>().each(
		    [&](const auto& entity, FSMAgentComponent& t)
		    { t.execute(static_cast<int>(entity)); }
		);
		return false;
	}
};

class AIHandler
{
    SceneHandler *sh = nullptr; 
    EventSystem ESys;

    public: 
    AIHandler() = default;
    // TODO: Map to store different FSMs... 
    //MovementFSM movementFSM;
	//std::vector<FSM*> FSMs;
	std::unordered_map<std::string, FSM*> FSMs;
	std::unordered_map<FSM*, std::function<void(FSM* fsm)>> FSMimguiLambdas;

    void addFSM(FSM* fsm, const std::string& name) { 
		fsm->init(this->sh->getScene(), &ESys, name);
        FSMs.insert({fsm->getName(), fsm});
    }

    void addImguiToFSM(const std::string& name, std::function<void(FSM* fsm)> imguiLambda) 
    {
        this->FSMimguiLambdas[this->FSMs[name]] = imguiLambda;		
    }

    // Init all FSM and BT related things
    void init(SceneHandler *sh)
    {
        this->sh = sh; 
        // Create System for FSM to be updated
        sh->getScene()->createSystem<FSM_system>(); //TODO: This should not be handled by a ECS system, but part of this class...
        

    }

    void createAIEntity(uint32_t entityID, FSM* fsm)
	{
		// Register entityID to a specific FSM (MovementFSM in this test)
		this->sh->getScene()->setComponent<FSMAgentComponent>(entityID, fsm);

		// Add required FSMComponents to entityID
		for (auto& requiredComp : fsm->getRequiredFSMComponents())
		{
			requiredComp->registerEntity(entityID, this->sh->getScene());
		}
		// Add required BTComponents to entityID
		for (auto& requiredComp : fsm->getRequiredBTComponents())
		{
			requiredComp->registerEntity(entityID, this->sh->getScene());
		}
		// Register this entity to all entity evenets of the FSM
		this->ESys.registerEntityToEvent(entityID, fsm);
	}

    void createAIEntity(uint32_t entityID, const std::string& fsmName)
	{
		createAIEntity(entityID, FSMs[fsmName]);
	}


    void drawImgui();

    void update(){ESys.update();drawImgui();}

};