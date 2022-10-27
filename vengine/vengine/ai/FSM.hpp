#pragma once
#include "BehaviorTree.hpp"
#include "EventSystem.hpp"//////////////////////////
#include "../dev/Log.hpp"
#include <functional>
#include <iostream>
#include <unordered_map>
#include <vector>


class FSM_Node
{
private:
public:
	FSM_Node() = default;
	FSM_Node(const std::string& status, BehaviorTree* bt) :
	    status(status), bt(bt){}

	std::string     status;
	BehaviorTree*   bt = nullptr;                    //TODO: Should not be public? 
	std::unordered_map<Event*, FSM_Node*> neighbors; //TODO: Should not be public?    

public:
	void execute(uint32_t entityID) 
    {
        if(bt){bt->execute(entityID);}else{Log::error("Failed: BT is a nullptr!");} 
    }

	void addNeighbor(EntityEvent* event, FSM_Node* node)
	{
		this->neighbors.insert({event, node});
	};
	void addNeighbor(GlobalEvent* event, FSM_Node* node)
	{
		this->neighbors.insert({event, node});
	};
    
};

class FSM	// TODO: Handlememory for New fsm_nodes and trees 
{
private: 
    std::unordered_map<std::string, FSM_Node*> fsm_nodes;
    std::unordered_map<std::string, BehaviorTree*> trees;
    
    FSM_Node* currentNode = nullptr;

	std::string name = "NONAME";

private: 
    void insertNode(std::string name)
    {
        fsm_nodes.insert({name, new FSM_Node(name, trees[name])});
    }

    //TODO: Make this part of class (Virtual), take template Type for type, 
    // template<typename T>
    // void regEntiy(uint32_t entityId, Scene* scene);
	   
protected:
    EventSystem* eventSystem; 
    static SceneHandler* sceneHandler; 

protected:
    struct BT_AND_NAME
    {
        std::string name;
        BehaviorTree* BT;
    };
    

    void addEntityTransition(std::string from, EntityEvent& transition, std::string to)
    {
        fsm_nodes[from]->addNeighbor(&transition, fsm_nodes[to]);
        this->eventSystem->registerEntityEvent(this, fsm_nodes[to], &transition);
    }
    void addGlobalTransition(std::string from, GlobalEvent& transition, std::string to)
    {
        fsm_nodes[from]->addNeighbor(&transition, fsm_nodes[to]);
        this->eventSystem->registerGlobalEvent(this, fsm_nodes[to], &transition);
    }

    void addBT(std::string name, BehaviorTree* BT) 
    {		
        // Initiating Tree to populate BTs requiredBTComponents!
        BT->startTree(this->sceneHandler,name); 

        this->trees.insert({name, BT});
        this->insertNode(name);
    }
    
    void addBTs(std::vector<BT_AND_NAME> BTs)
    {
        for (auto bt : BTs)
        {
            // Initiating Tree to populate BTs requiredBTComponents!
            bt.BT->startTree(this->sceneHandler,bt.name);

            this->trees.insert({bt.name, bt.BT});
            this->insertNode(bt.name);
        }
        
    }
    void setInitialNode(std::string node) 
    {
        this->currentNode = this->fsm_nodes[node];
    }

    template<typename T>
    void addRequiredComponent(uint32_t entityID)
	{
        sceneHandler->getScene()->setComponent<T>(entityID);
    }

    virtual void registerEntityComponents(uint32_t entityId) = 0;

    virtual void real_init() = 0; 

public:

    //TODO: Make this part of class (Virtual), take template Type for type, 
    void init(SceneHandler* sh, EventSystem* es, const std::string& name) 
    {
		this->name = name;

        this->sceneHandler = sh; 
        this->eventSystem = es; 
		real_init();
		for (auto& tree : trees)
		{
			tree.second->startTree(sh,name);// TODO: Is this correct, or is it done during addBT func? ... Should be removed(?)
		}
        checkForCommonErrors();
    }

    void registerEntity(uint32_t entityId)
    {
        // Add Components that FSM needs
        registerEntityComponents(entityId);

        // Add Components that BT needs
        for(auto& tree : this->trees)
        {
            tree.second->registerEntity(entityId);
        }        
    }

    void clean()
	{
        for (auto p : fsm_nodes)
		{
		    delete p.second;
        }
		for (auto p : trees)
		{
		    delete p.second;
        }
    }

    void checkForCommonErrors()
    {
        int foundProblem = 0; 
        if(this->currentNode == nullptr)
        {
            Log::warning("Initial Node is nullptr! In real_init(); set starting node with setInitalNode()");
            foundProblem++;
        }
        if(this->trees.size() < 1)
        {
            Log::warning("FSM needs atleast one BehaviorTree to work! In real_init(); add a BT with addBT()");
            foundProblem++;
        }
        if(checkForDeadEndNode() && this->trees.size() != 1)
        {
            Log::warning("FSM Contains atleast one node that cannot be reached or cannot reach any other node! In real_init(); use add***Transition() so that all nodes can be traversed to and from!");
            foundProblem++;
        }
        if(foundProblem != 0){Log::error("FSM["+this->name+"] is invalid, found "+std::to_string(foundProblem)+" problems! (See Warnings Above!)");}
    }
    bool checkForDeadEndNode()
    {
        // All nodes MUST be able to be traversed FROM and TO by ATLEAST one other node! 
        bool foundDeadEnd = false;
        struct TransitionToAndFrom
        {
            bool from   = false;
            bool to     = false;
            bool valid(){return to || from;}
        };
        std::unordered_map<std::string, TransitionToAndFrom> treeTransitions;
        std::unordered_map<BehaviorTree*, std::string> treeToName;

        
        //temporary use a map<BT*,string> in order to remember each name for each bt, used later!        
        for(auto& fsmNodeTree : this->trees)
        {            
            treeToName[fsmNodeTree.second] = fsmNodeTree.first;
        }

        // For all Trees (i.e. FSM Node), check if it has atleast one transition FROM the node, and one transition TO the node        
        for(auto& fsmNodeTree : this->trees)
        {            
            if(this->fsm_nodes.count(fsmNodeTree.first) > 0)
            {        
                // Node has neighbors, thus we have a transition from node exists!
                treeTransitions[fsmNodeTree.first].from = true; 
                                    
                // loop through all neighbors, i.e. nodes that has a transition TO them...
                for(auto neigbhor : this->fsm_nodes[fsmNodeTree.first]->neighbors)
                {
                    // Get name for the fsm_nodes BT
                    std::string nameOfBT = treeToName[neigbhor.second->bt];
                    // This node can be transitioned TO...
                    treeTransitions[nameOfBT].to = true; 
                }
            }            
        }        
        
        for(auto tree : treeTransitions)
        {
            if(!tree.second.valid())
            {
                if(!tree.second.from)
                {
                    Log::warning("FSM transition FROM bt["+tree.first+"] was not found!");
                }
                if(!tree.second.to)
                {
                    Log::warning("FSM transition TO bt["+tree.first+"] was not found!");
                }
                foundDeadEnd = true;                
            }
        }

        return foundDeadEnd;
    }


    const std::string& getName()const { return this->name;}
	void setCurrentNode(FSM_Node* node) {
		this->currentNode = node;
	}
	void setCurrentNode(std::string node)
	{
		this->currentNode = this->fsm_nodes[node];
	}
	FSM_Node* getCurrentNode() const {
		return this->currentNode;
	}

	void drawBT(std::string tree) { 
		this->trees[tree]->draw();
	}
	
    FSM()=default;
	FSM(SceneHandler* sceneHandler, EventSystem* eventSystem) 
		:
	    eventSystem(eventSystem)
	{
        //NOTE: Cant call Pure Virtual function from constructor...
		//startInit();
		//init();

		if (FSM::sceneHandler != sceneHandler)
		{
			FSM::sceneHandler = sceneHandler;
		}
	};
	

	
	std::vector<FSM_Node*> nodes;

	void execute(uint32_t entityID);
};