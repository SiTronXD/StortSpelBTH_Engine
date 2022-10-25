#include "BehaviorTree.hpp"
#include "../components/BTComponent.hpp"
#include <stack>
#include <fstream>
#include <iostream>

Scene* BehaviorTree::scene = nullptr;

void BehaviorTree::draw()
{
    std::stack<Node*> nodes;
    nodes.push(this->root);

    std::ofstream out("graph.dot");
    out << "digraph {\n" ;

    while(!nodes.empty())
    {
        auto currentNode = nodes.top();
        nodes.pop();        

        // Nodes in dot-language must have unique names  
        out << "Node_"<< currentNode->getID() <<" [\n";
        
        // Set name of the node
        out << "label = \""<< currentNode->displayName << "\"\n"; // No need to pollute display name//  <<"_"<< currentNode->getID() ;

        // Set
        if(currentNode->nodeType != NodeType::Condition){
            out << "shape=record\n";
            if(currentNode->nodeType == NodeType::Task){
                out << "style = filled\n";
                out<< "fillcolor=gray\n";
            }
        }
        else{
            out << "shape=oval\n";
        }        
        

        // Draw return value indicator for node 
        switch (currentNode->lastReturn) {
            case BTStatus::Success :
                out<< "xlabel=<<font color=\"darkgreen\">↑</font>>";
                break;
            case BTStatus::Running :
                out<< "xlabel=<<font color=\"orange\">↑</font>>";
                break;
            case BTStatus::Failure :
                out<< "xlabel=<<font color=\"red\">↑</font>>";
                break;
            case BTStatus::INVALID :
                //assert(false && "Invalid BTStatus return");
                out<< "xlabel=<<font color=\"blue\">🛑</font>>";
                break;
        }

        // Set Potential Decorators of the node

        // End node label
        out << "\n" << "]\n";
                

        for(auto& node : currentNode->children)
        {        
            out << "Node_"<< currentNode->getID() << "-> Node_"<< node->getID() << "\n";            
            nodes.push(node);            
        }       
        currentNode->lastReturn = BTStatus::INVALID;
    }
    out << "}\n";
    out.close();
}

Node::Node(NodeType nodeType, std::string &&typeName, std::string displayName)
    : uniqueID(getUniqueID()), lastReturn(BTStatus::INVALID),
      typeName(typeName), displayName(displayName), nodeType(nodeType) {
}


BTStatus Tag::decorate(uint32_t entityID)
{
	
	BTStatus ret = this->children[0]->execute(entityID);
	if (ret == BTStatus::Success || ret == BTStatus::Running)
	{
		//Set tag to true
        BehaviorTree::scene->
            getComponent<BTAgentComponent>(entityID)
                .setTag(this->tag, this, true);
	}
	else
	{
		//Set tag to false
        BehaviorTree::scene->
            getComponent<BTAgentComponent>(entityID)
                .setTag(this->tag, this, false);
	}
    lastReturn = ret;
	return ret;
}
