#pragma once 

#include "../ai/BehaviorTree.hpp"
#include <cstdint>
#include <unordered_map>

struct BTAgentComponent
{
    BehaviorTree* bt;   
    std::unordered_map<Node*,TagData>  tags;
    void setTag(std::string name, Node* nodeptr, bool status)
    {   
        this->tags[nodeptr] = {name, status};
    }

    BTAgentComponent(BehaviorTree* bt)
    : bt(bt)
    {}

    void execute(uint32_t entityId)
    {
        
        bt->execute(entityId);
    }
};
