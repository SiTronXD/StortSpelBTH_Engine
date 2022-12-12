#pragma once


#include "BehaviorTreeNodes.hpp"
#include "BTCreateHelper.hpp"
#include "AgentStatus.hpp"
#include "../application/SceneHandler.hpp"
#include "../dev/Log.hpp"
#include <array>
#include <cstdint>
#include <vector>
#include <string>
#include <cassert>
#include <functional>
#include <unordered_map>
#include <iostream>
#include <stack>

// Decorator 
////{ Is a property of a Node!  }

// Activation/Deactivation : Decorator

// Modify Termination status : Decorator

// Parallel execution : Decorator ( ?? Node Or Decorator? Probalby Node...)

// Custom (i.e. Addon) : Decorator

// [Debug tool] Tag : Decorator 

// [Debug tool] Message : Decorator  

// [Debug tool] Breakpoint : Decorator  


// Node 
//  - Decorator 

// Sequence : Node

// Selector/Fallback : Node

// Parallel : Node 

// Reference : Node
////{ Used to split up a Tree into subtree, a reference to the root of such subtree }

// Task : Node

// Performance
//// - Use a bitvector for status of all a Trees Conditional nodes
//// - Use multiple Threads if needed when doing the bitvector checking
//// - Seperate Trees into into different groups based on their needs;
//// - - Trees that only needs to be updated if a Event happened; This can be done by checking if the values of a bitvector has changed...
//// - - Trees that needs to be updated More frequently but not not too critical. ex, once every 0.5 sec
//// - - Trees that needs to be updated very frequently but only after a special event is triggered. ex, less than times per 0.5 sec
//// - - Trees update rate can also be dynamic (Active Sensing), by having a varying value that specifies of often it should update. And let this value depend on how critical the current situation is.
#include "PathFinding.h"

class BehaviorTree {
private:
    std::string name;
    Node* root =nullptr;
    
protected:
    virtual void start() = 0;

    bool started = false;
    friend class Tag;
    static SceneHandler* sceneHandler;
    static SceneHandler* getSceneHandler(){return BehaviorTree::sceneHandler;};
    void setRoot(Node* root) {this->root = root;};

    virtual void registerEntityComponents(Entity entityId) = 0;

    template<typename T>
    void addRequiredComponent(Entity entityID)
	{
        sceneHandler->getScene()->setComponent<T>(entityID);
    }
    // wrapper for creating different kind of nodes, this will help with debugging and memory management! 
        
    BTCreateHelper create;       //Shorthand for Creates
    BTCreateHelper&  c = create; //Shorthand for Creates
    
public:     
    virtual ~BehaviorTree(){
        std::stack<Node*> stack;

        // If BT was created without AIHandler, then root could be nullptr. 
        if(!this->root){return;} 

        stack.push(this->root);
        while(!stack.empty())
        {
            Node* current = stack.top(); 
            stack.pop();
            for(auto child : current->children)
            {
                stack.push(child);
            }
            delete current;
            
        }
    }

    inline const std::string& getName()const { return this->name;}
    
    //Only run start function one time... (Start is for everything in the tree)
	void startTree(SceneHandler* sceneHandler, const std::string& name)
	{
        this->name = name;

		if (!this->started)
		{
			this->start();
		}
        checkForCommonErrors();
		this->started = true;
		this->sceneHandler = sceneHandler;
	};

    void checkForCommonErrors()
    {
        int foundProblem = 0; 
        if(this->root == nullptr)
        {
            Log::warning("root node is nullptr! In start(); set root node with setRoot()");
            foundProblem++;
        }
        if(checkForNoNodes())
        {
            Log::warning("No nodes added to the tree! In start(); define nodes to be added with create.<type>.<node>");
            foundProblem++;
        }
        else
        {
            if(create.leaf.getNrOfRegistredLeafs() == 0)
            {
                Log::warning("No leaf nodes defined in tree! In start(); define nodes to be added with create.leaf.<node>");
                foundProblem++;
            }
        }
        if(checkForCompositorWithoutChildren()) 
        {
            Log::warning("All compositor nodes need to have atleast one child! In start(); make sure all Compositor types have run addLeaf/s with atleast 1 node each.");
            foundProblem++;
        }
        if(checkForDecoratorsWithoutChild()) 
        {
            Log::warning("Decorator nodes must have One child! In start(); make sure all decorators types have run addLeaf with exactly 1 node each.");
            foundProblem++;
        }
        if(checkForDecoratorsWithMoreThanOneChild()) 
        {
            Log::warning("Decorator nodes are not allowed to have more than one Child! In start(); make sure all decorators types does not run addLeaf for more than 1 node each.");
            foundProblem++;
        }
        if(checkForLeafNodesWithChildren()) 
        {
            Log::warning("Leaf node cannot have a Child node! In start(); make sure no leaf node runs addLeaf/s, addCompositor/s, addDecorator/s, addFallback/s or addChildren.");
            foundProblem++;
        }
        if(checkForDisconnectedTree()) 
        {
            Log::warning("Atleast one node is not part of the tree (a non-root node has no parent)! In start(); Make sure all nodes has some indirect connection to the root node!");
            foundProblem++;
        }
        if(foundProblem != 0){Log::error("BehaviorTree["+this->name+"] is invalid, found "+std::to_string(foundProblem)+" problems! (See Warnings Above!)");}
    }
    bool checkForNoNodes()
    {        
        return create.compositor.getNrOfRegistredCompositors() == 0 &&
               create.leaf.getNrOfRegistredLeafs() == 0 &&
               create.decorator.getNrOfRegistredDecorators() == 0; 
    }
    bool checkForCompositorWithoutChildren()
    {        
        bool someCompositorHasNoChildren = false;
        for(auto compositor : create.compositor.getCompositors())
        {
            if(compositor->children.size() == 0)
            {
                someCompositorHasNoChildren = true; 
                break;
            }
        }
        return someCompositorHasNoChildren;
    }
    bool checkForDecoratorsWithoutChild()
    {        
        bool someDecoratorHasNoChild = false;
        for(auto decorator : create.decorator.getDecorators())
        {
            if(decorator->children.size() == 0)
            {                
                someDecoratorHasNoChild = true; 
                break;
            }
        }
        return someDecoratorHasNoChild;
    }
    bool checkForDecoratorsWithMoreThanOneChild()
    {        
        bool someDecoratorHasMoreThanOneChild = false;
        for(auto decorator : create.decorator.getDecorators())
        {
            if(decorator->children.size() > 1 && decorator->nodeType != NodeType::Random)
            {
                someDecoratorHasMoreThanOneChild = true; 
                break;
            }
        }
        return someDecoratorHasMoreThanOneChild;
    }
    bool checkForLeafNodesWithChildren()
    {        
        bool someLeafNodeHasAtleastOneChild = false;
        for(auto leaf : create.leaf.getLeafs())
        {
            if(leaf->children.size() > 0)
            {
                someLeafNodeHasAtleastOneChild = true; 
                Log::warning("LeafNode["+leaf->displayName+"] has a child!");
            }
        }
        return someLeafNodeHasAtleastOneChild;
    }
    bool checkForDisconnectedTree()
    {        
        bool treeIsDisconnected = false;
        for(auto leaf : create.leaf.getLeafs())
        {
            if(leaf->parent == nullptr && (leaf != this->root || this->root == nullptr))
            {
                Log::warning("LeafNode["+leaf->displayName+"] has no parent!");
                treeIsDisconnected = true;                 
            }
        }
        for(auto decorator : create.decorator.getDecorators())
        {
            if(decorator->parent == nullptr && (decorator != this->root || this->root == nullptr))
            {                
                Log::warning("decoratorNode["+std::to_string(decorator->uniqueID)+"] has no parent!");
                treeIsDisconnected = true;                 
            }
        }
        for(auto compositor : create.compositor.getCompositors())
        {
            if(compositor->parent == nullptr && (compositor != this->root || this->root == nullptr))
            {
                treeIsDisconnected = true;     
                Log::warning("compositorNode["+std::to_string(compositor->uniqueID)+"] has no parent!");
            }
        }
        
        return treeIsDisconnected;
    }

    void execute(Entity entityID){ root->execute(entityID);};

    void registerEntity(Entity entityId)
    {
        registerEntityComponents(entityId);        
    }

    // Debug
    void draw();
};

