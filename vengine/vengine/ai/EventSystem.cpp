#include "EventSystem.hpp"
#include "FSM.hpp"
#include "../components/FSMAgentComponent.hpp"
#include "../dev/Log.hpp"

//void registerEvent(FSM_Node* who, Event* event)

void EventSystem::registerGlobalEvent(
    FSM* fsm, FSM_Node* who, GlobalEvent* event
)
{
	// TODO: Replace vector with map to get better performance...
	bool found = false;
	for (auto& s : this->globalSubscribers[event])
	{
		if (s.node == who)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		this->globalSubscribers[event].push_back({fsm, who});
	}
	else
	{
		std::cout << "Warning! Node was already subscribed to event! \n";
	}

	//TODO: might not be a good idea to use bool, first time its neither true or false. Use enum?
	this->globalEventLastReturn[event] = false;
}

void EventSystem::registerEntityEvent(
    FSM* fsm, FSM_Node* who, EntityEvent* event
)
{
	// TODO: Replace vector with map to get better performance...
	bool found = false;
	for (const auto& s : this->entityEvents[event])
	{
		if (s.node == who)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{	
		this->entityEvents[event].push_back({fsm, who});
		this->fsmEvents[fsm].push_back({event, who});
	}
	else
	{
		std::cout << "Warning! Node was already subscribed to event! \n";
	}

	//TODO: might need to implement this later (Active sensing)
	//eventLastReturn[event] = false;
}

void EventSystem::update()
{	
    if(this->currentScene != this->sh->getScene())
    {
        this->entitySubscribers.clear();
        this->entityEventLastReturn.clear();
    }

	for (auto& event : this->globalSubscribers)
	{

		if (event.first->checkEvent())
		{
			if (!this->globalEventLastReturn[event.first])
			{
				for (auto subscriber : event.second)
				{
					//TODO: What will happen, how can I call whatever func my subscriber want me to call?
					subscriber.fsm->setCurrentNode(subscriber.node);
				}
				std::cout << event.second[0].fsm->getCurrentNode()->status
				          << std::endl;
				this->globalEventLastReturn[event.first] = true;
			}
		}
		else if (this->globalEventLastReturn[event.first])
		{
			this->globalEventLastReturn[event.first] = false;
		}
	}	

	//TODO: Move to separate update funtion
    std::vector<std::string*> activeEntityEventNames; 
	// For All Entities ...
	for (auto& eventsPerEntity : this->entitySubscribers)
	{
		const auto& entityID = eventsPerEntity.first;
		const auto& entityEventPtrs = eventsPerEntity.second;

        
		auto& agent = this->sh->getScene()->getComponent<FSMAgentComponent>(entityID);

		for (auto& currNeighbor : agent.currentNode->neighbors)
		{
			auto* eventTransition = (EntityEvent*)currNeighbor.first;			

			if (eventTransition->checkEvent(entityID))
			{
                activeEntityEventNames.emplace_back(&eventTransition->getName());
                
				if (!this->entityEventLastReturn[entityID][eventTransition])
				{
					// TODO: Maybe currentEvent could be held by the FSMComponent?
					// NOTE: Unsure, but I think we have multiple instances of FSMs right now?...

					//If event was true, update the current Entitys FSM
					agent.fsm->setCurrentNode(currNeighbor.second);
					agent.currentNode = currNeighbor.second;

					std::cout << agent.currentNode->status
								<< std::endl;

					this->entityEventLastReturn[entityID][eventTransition] = true;
				}                
			}
			else if (this->entityEventLastReturn[entityID][eventTransition])
			{
				this->entityEventLastReturn[entityID][eventTransition] = false;
			}

            if(activeEntityEventNames.size() > 1 )
            {
                std::string activeEventsStr; 
                size_t c = 0; 
                for(auto& e:activeEntityEventNames){
                    if(c != activeEntityEventNames.size() -1)
                    {activeEventsStr += *e+", ";}
                    else {activeEventsStr += *e;}
                    c++;
                }
                Log::warning("Entity["+std::to_string(entityID)+"] More than one FSM Transition Events {"+activeEventsStr+"} are active, will cause problems");
            }

		}
        activeEntityEventNames.clear();
	}
}
