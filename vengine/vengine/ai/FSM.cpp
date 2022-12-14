#include "pch.h"
#include "FSM.hpp"
#include "../components/FSMAgentComponent.hpp"

SceneHandler* FSM::sceneHandler = nullptr;

void FSM::execute(Entity entityID) 
{ 
	FSM_Node* node = sceneHandler->getScene()->getComponent<FSMAgentComponent>(entityID).currentNode;
	if(node)
	{
		node->execute(entityID);
	}
	else
	{
		Log::error("Failed: FSM_Node is a nullptr!");
	} 
}
