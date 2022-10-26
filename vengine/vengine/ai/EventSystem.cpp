#include "EventSystem.hpp"
#include "FSM.hpp"
#include "../components/FSMAgentComponent.hpp"

//void registerEvent(FSM_Node* who, Event* event)

void EventSystem::registerGlobalEvent(
    FSM* fsm, FSM_Node* who, GlobalEvent* event
)
{
	// TODO: Replace vector with map to get better performance...
	bool found = false;
	for (auto& s : globalSubscribers[event])
	{
		if (s.node == who)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		globalSubscribers[event].push_back({fsm, who});
	}
	else
	{
		std::cout << "Warning! Node was already subscribed to event! \n";
	}

	//TODO: might not be a good idea to use bool, first time its neither true or false. Use enum?
	globalEventLastReturn[event] = false;
}

void EventSystem::registerEntityEvent(
    FSM* fsm, FSM_Node* who, EntityEvent* event
)
{
	// TODO: Replace vector with map to get better performance...
	bool found = false;
	for (const auto& s : entityEvents[event])
	{
		if (s.node == who)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		//entitySubscribers[entityID].push_back({fsm, who, event});
		entityEvents[event].push_back({fsm, who});
		fsmEvents[fsm].push_back({event, who});
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
	for (auto& event : globalSubscribers)
	{

		if (event.first->checkEvent())
		{
			if (!globalEventLastReturn[event.first])
			{
				for (auto subscriber : event.second)
				{
					//TODO: What will happen, how can I call whatever func my subscriber want me to call?
					//subscriber.fsm->currentNode = subscriber.node;
					subscriber.fsm->setCurrentNode(subscriber.node);
				}
				std::cout << event.second[0].fsm->getCurrentNode()->status
				          << std::endl;
				globalEventLastReturn[event.first] = true;
			}
		}
		else if (globalEventLastReturn[event.first])
		{
			globalEventLastReturn[event.first] = false;
		}
	}	

	//TODO: Move to separate update funtion
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
				if (!entityEventLastReturn[entityID][eventTransition])
				{
					// TODO: Maybe currentEvent could be held by the FSMComponent?
					// NOTE: Unsure, but I think we have multiple instances of FSMs right now?...

					//If event was true, update the current Entitys FSM
					agent.fsm->setCurrentNode(currNeighbor.second);
					agent.currentNode = currNeighbor.second;

					std::cout << agent.fsm->getCurrentNode()->status
								<< std::endl;

					entityEventLastReturn[entityID][eventTransition] = true;
					//break; 
				}
			}
			else if (entityEventLastReturn[entityID][eventTransition])
			{
				entityEventLastReturn[entityID][eventTransition] = false;
			}

		}

		//// Loop through their registred eventPtr... (eventPtr used to index into this->entityEvents)
		//for (auto entityEventPtr : entityEventPtrs)
		//{
		//	// Check Event function for the current Entity!
		//	for (auto& entityEvent : entityEvents[entityEventPtr])
		//	{

		//		// Check Event for Entity with given entityID
		//		if (entityEventPtr->checkEvent(entityID))
		//		{
		//			if (!entityEventLastReturn[entityID][entityEventPtr])
		//			{
		//				// TODO: Maybe currentEvent could be held by the FSMComponent?
		//				// NOTE: Unsure, but I think we have multiple instances of FSMs right now?...

		//				//If event was true, update the current Entitys FSM
		//				entityEvent.fsm->setCurrentNode(entityEvent.node);

		//				std::cout << entityEvent.fsm->getCurrentNode()->status
		//				          << std::endl;

		//				entityEventLastReturn[entityID][entityEventPtr] = true;
		//			}
		//		}
		//		else if (entityEventLastReturn[entityID][entityEventPtr])
		//		{
		//			entityEventLastReturn[entityID][entityEventPtr] = false;
		//		}
		//	}
		//}
	}
}
