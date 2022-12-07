#pragma once 
 #include "op_overload.hpp"
#include "BehaviorTree.hpp"

#include <functional>
#include <iostream>
#include <unordered_map>
#include <vector>

// Forward declaration

class FSM_Node;
class FSM;
class EntityEvent;

 struct FSM_Events // Used to get all events belonging to a fsm! 
{
    EntityEvent* event; 
    FSM_Node* node; 
};
struct FSM_and_Node
{
	FSM* fsm;
	FSM_Node* node;
};

struct FSM_and_Node_and_Event
{
	FSM* fsm;
	FSM_Node* node;
	EntityEvent* event;
};

class Event
{
private:
    std::string name; 
public:
	Event(const std::string& name) : name(name){};
	virtual ~Event() {};
    inline std::string& getName(){return name;};
};
class GlobalEvent : public Event
{
private:
public:
	std::function<bool()> event;
	bool checkEvent() { return event(); }
	GlobalEvent() = default; //TODO: Remove
	GlobalEvent(const std::string& name, std::function<bool()> event) :Event(name), event(event){};
};

class EntityEvent : public Event
{
   private:
   public:
	std::function<bool(uint32_t)> event;    
	bool checkEvent(uint32_t entityID) { return event(entityID); }
	EntityEvent() = default; //TODO: Remove
	EntityEvent(const std::string& name, std::function<bool(uint32_t)> event) :Event(name), event(event){};
};

class EventSystem
{
private:
	std::unordered_map<GlobalEvent*, bool> globalEventLastReturn;
	std::unordered_map<GlobalEvent*,   std::vector<FSM_and_Node>> globalSubscribers;

	//TODO: Entity event last returned (Active sensing)
	std::unordered_map<uint32_t, std::unordered_map<EntityEvent*, bool>> entityEventLastReturn;
	std::unordered_map<uint32_t, std::vector<EntityEvent*>> entitySubscribers;
	std::unordered_map<EntityEvent*, std::vector<FSM_and_Node>> entityEvents;


    std::unordered_map<FSM*, std::vector<FSM_Events>> fsmEvents;

	SceneHandler* sh;
public:
	void setSceneHandler(SceneHandler* sh) {this->sh = sh; this->currentScene = this->sh->getScene();}
	void registerGlobalEvent(FSM* fsm, FSM_Node* who, GlobalEvent* event);
	void registerEntityEvent(FSM* fsm, FSM_Node* who, EntityEvent* event);

    void clean(){
        entitySubscribers.clear();
        entityEventLastReturn.clear();
    }

    void registerEntityToEvent(uint32_t entityID, FSM* fsm)
    {
        for(auto fsmEvent : this->fsmEvents[fsm])
        {
            entitySubscribers[entityID].push_back(fsmEvent.event);
			entityEventLastReturn[entityID][fsmEvent.event] = false;
        }

    }
	void resetEntityLastReturn()
	{
		for(auto& elr: this->entityEventLastReturn)
		{
			for(auto& ev: elr.second)
			{
				ev.second = false;
			}
		}
	}

	Scene* currentScene = nullptr; //TODO make const...

	void update();
};
