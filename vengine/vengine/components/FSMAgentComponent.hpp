#pragma once

#include <cstdint>
//#include <unordered_map>
#include "../ai/FSM.hpp"

struct FSMAgentComponent
{
	FSM* fsm;
	/*std::unordered_map<Node*, TagData> tags;
	void setTag(std::string name, Node* nodeptr, bool status)
	{
		this->tags[nodeptr] = {name, status};
	}*/

	FSMAgentComponent(FSM* fsm) : fsm(fsm) {}

	void execute(uint32_t entityId) { fsm->execute(entityId); }
};
