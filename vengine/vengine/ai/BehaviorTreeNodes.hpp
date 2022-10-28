#pragma once 
#include "../application/Scene.hpp"
#include "../dev/Log.hpp"
#include <array>
#include <cstdint>
#include <vector>
#include <string>
#include <cassert>
#include <functional>
#include <unordered_map>
#include <iostream>


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <intrin.h>
#define call_breakpoint() __debugbreak()
#else
#define call_breakpoint() __builtin_debugtrap()
#endif 


class Task;

enum class BTStatus {
    INVALID,
    Success,
    Running,
    Failure
    
};

enum class NodeType {
    INVALID,
    Selector,
    Sequence,
    Parallel,
    Condition,
    Reference,
    Task,
    Tag,
    Message,
    Invert,
    ForceSuccess,
    ForceFailure,
    Repeat,
    Retry,
    Breakpoint
};




class BehaviorTree;
class Leaf;
class Decorator;
class Compositor;
class Condition;
struct Fallback;

class Node {
private: 
    friend BehaviorTree;
    const uint32_t uniqueID;
    
protected:
    Node* parent = nullptr;
    std::vector<Node*> children;
    // std::vector<Decorator> decorators; // TODO: we will treat these as regular nodes, easier to implement
    std::vector<Leaf*> leafs;   
    std::vector<Decorator*> decorators;   
    BTStatus lastReturn;
    
    // Representation data
    const std::string typeName;
    const std::string displayName;
    inline uint32_t getUniqueID() { static uint32_t uniqueID = 0; uniqueID++; return uniqueID;};

public:
    virtual ~Node()=default;
    const NodeType nodeType = NodeType::INVALID; // TODO: Check if this crash on Windows...
    Node(NodeType nodeType, std::string &&typeName, std::string displayName);
    
    // TODO: Check if we should refactor adding of Nodes to Compositor class...

    // Common function to add different Node types in the same call...
    inline void addChildren(std::vector<Node*> nodes);

    inline void addCompositor(Compositor* compositor);
    inline void addCompositors(std::vector<Compositor*> compositors);

    inline void addFallback( Fallback&& fallback);
    inline void addFallbacks(std::vector<Fallback> fallbacks);

    inline void addLeaf(Leaf* task);
    inline void addLeafs(std::vector<Leaf*> tasks);

    inline void addDecorator(Decorator* decorator);    

    virtual BTStatus execute(uint32_t entityID) = 0;

    inline const uint32_t& getID(){return uniqueID;};
};


//Compositor Interface
class Compositor : public Node{
private:
public:
    virtual ~Compositor()=default;
    Compositor(NodeType nodeType, std::string &&typeName, std::string displayName) 
    : Node(nodeType, std::move(typeName), displayName) {};    
};


class Selector : public Compositor{
private:
public:     
    Selector() : Compositor(NodeType::Selector, "Selector", "?") {};

    BTStatus execute(uint32_t entityID) override 
    {        
        for(auto& child : this->children)
        {
            BTStatus status = child->execute(entityID);
            if(status == BTStatus::Running)
            {
                lastReturn = BTStatus::Running;
                return BTStatus::Running;
            }
            else if(status == BTStatus::Success)
            {
                lastReturn = BTStatus::Success;
                return BTStatus::Success;
            }
        }
        lastReturn = BTStatus::Failure;
        return BTStatus::Failure;
    };
};

class Sequence : public  Compositor{
private:
public:
    Sequence() : Compositor(NodeType::Sequence, "Sequence", "→") {};

    BTStatus execute(uint32_t entityID) override 
    {
        for(auto& child : this->children)
        {
            BTStatus status = child->execute(entityID);
            if(status == BTStatus::Running)
            {
                lastReturn = BTStatus::Running;
                return BTStatus::Running;
            }
            else if(status == BTStatus::Failure)
            {
                lastReturn = BTStatus::Failure;
                return BTStatus::Failure;
            }
        }
        lastReturn = BTStatus::Success;
        return BTStatus::Success;
    };

};

// TODO: Think about how to make Parallel useful...
class Parallel : public  Compositor{
private:
public:
    Parallel() : Compositor(NodeType::Parallel, "Prallel", "⇉") {};

    BTStatus execute(uint32_t entityID) override 
    {
        BTStatus ret = BTStatus::Success;
        for(auto& child : this->children)
        {
            BTStatus status = child->execute(entityID);
            if(status == BTStatus::Failure)
            {
                ret = BTStatus::Failure;
            }
        }
        lastReturn = ret;
        return ret;
    };

};

class Reference : public Compositor
{
   private:
	    Node* referenceNode;
   public:
	    Reference(Node* ref = nullptr) :
	    Compositor(NodeType::Reference, "Reference", "Ref"), referenceNode(ref){}
	        

	BTStatus execute(uint32_t entityID) override
	{

        if (this->referenceNode == nullptr)
		{
			lastReturn = BTStatus::Failure;
			return BTStatus::Failure;
		}

        lastReturn = this->referenceNode->execute(entityID);
		return lastReturn;
	};

    void Setreference(Node* ref) {
        this->referenceNode = ref;
    }
};

//Leaf Interface
class Leaf : public Node{
private:
public:
    virtual ~Leaf()=default;
    Leaf(NodeType nodeType, std::string &&typeName, const std::string& displayName) 
    : Node(nodeType, std::move(typeName), displayName) {};    
};

class Condition : public Leaf{
private:
public:

    Condition(const std::string& description, std::function<BTStatus(uint32_t)> conditional) 
    : Leaf(NodeType::Condition, "Condition", description), conditional(conditional) {};

    BTStatus execute(uint32_t entityID) override 
    {
        lastReturn = conditional(entityID);
        return lastReturn;
    };
    std::function<BTStatus(uint32_t)> conditional;
    
};



class Task : public Leaf {
    
    BTStatus doStartRet = BTStatus::INVALID;
    std::unordered_map<uint32_t, BTStatus> entityStates; // TODO: Make sure BTStatus is 0 as default...
public:

    Task(const std::string& description, std::function<BTStatus(uint32_t)> process , std::function<void(uint32_t)> start = nullptr)  //TODO: Check this, why does Task have 3 params, we should only use 2! 
    :   Leaf(NodeType::Task, "Task", description), 
        processFunc(process),
        startFunc(start){};
    
    void start(uint32_t entityID) 
    {
       startFunc(entityID);
    }
    BTStatus execute(uint32_t entityID) override 
    {
        // TODO: Make use of this later! 
        if(entityStates[entityID] != BTStatus::Running && startFunc != nullptr){
            start(entityID);
        }
        lastReturn = processFunc(entityID);
        doStartRet = lastReturn;
        entityStates[entityID] = lastReturn;
        return lastReturn;
    }
    std::function<BTStatus(uint32_t entityID)>  processFunc;
    std::function<void(uint32_t entityID)>      startFunc;
};

//Decorator Interface
class Decorator : public Node{
private:
public:
    virtual ~Decorator()=default;
    
    Decorator(NodeType nodeType, std::string &&typeName, std::string displayName) 
    : Node(nodeType, std::move(typeName), displayName) {};    

    BTStatus execute(uint32_t entityID)
    {
		if (this->children.size() > 1 || this->children.size() < 1)
		{
            Log::error("Decorators can only have one child!");		
			return BTStatus::INVALID;
		}
        // TODO: Might be unncessary with a decorate function, but might also help readability...
        return decorate(entityID);
    }
    virtual BTStatus decorate(uint32_t entityID) = 0;
};


struct TagData
{
    std::string tagname;
    bool active;
};

class Tag : public Decorator{
private:
public:
    Tag(const std::string& tag) 
    : Decorator(NodeType::Tag, "Tag", tag), tag(tag) {};

    std::string tag;     

    //Input some string.
    //This string is stored as a variable(bool?)(in blackboard? or in entities?).
    //This variable is only active then the node is active
	BTStatus decorate(uint32_t entityID);
};

class Invert : public Decorator{
private:
public:
    Invert() 
    : Decorator(NodeType::Invert, "Invert", "!"){};


    BTStatus decorate(uint32_t entityID)
    {
        
        BTStatus ret = this->children[0]->execute(entityID);
        if(ret == BTStatus::Success){
            return BTStatus::Failure;
        }
        else if(ret == BTStatus::Failure){
            return BTStatus::Success;
        }
        else{
            return ret;
        }
    }
};

class Message : public Decorator
{
   private:
	BTStatus curStatus;
	std::string msg;
   public:
	Message(std::string message) : //TODO: Check if message can be const std::string&.  
	    Decorator(NodeType::Message, "Message", "\"\""),
	    curStatus(BTStatus::INVALID),
	    msg(message){};

	BTStatus decorate(uint32_t entityID)
	{
		lastReturn = this->children[0]->execute(entityID);
		if (lastReturn != curStatus)
		{
			curStatus = lastReturn;
			std::cout << "Message: " << this->msg << std::endl;

        }
		return lastReturn;
	}
};

class ForceSuccess : public Decorator
{
   private:
   public:
	ForceSuccess() : Decorator(NodeType::ForceSuccess, "ForceSuccess", ":)"){};

	BTStatus decorate(uint32_t entityID)
	{ 
        lastReturn = this->children[0]->execute(entityID);
        return BTStatus::Success;
	}
};

class ForceFailure : public Decorator
{
   private:
   public:
	ForceFailure() : Decorator(NodeType::ForceFailure, "ForceFailure", ":("){};

	BTStatus decorate(uint32_t entityID)
	{
		lastReturn = this->children[0]->execute(entityID);
		return BTStatus::Failure;
	}
};

class Repeat : public Decorator
{
   private:
	int num_repeats;
   public:
	Repeat(int n = 1) :
	    Decorator(NodeType::Repeat, "Repeat", "repeat"), num_repeats(n){};

	BTStatus decorate(uint32_t entityID)
	{
		for (int i = 0; i < this->num_repeats; i++)
		{
		    lastReturn = this->children[0]->execute(entityID);
			if (lastReturn != BTStatus::Success)
			{
				break;
            }
        }
		return lastReturn;
	}
};

class Retry : public Decorator
{
   private:
	int num_repeats;

   public:
	Retry(int n = 1) :
	    Decorator(NodeType::Retry, "Retry", "retry"), num_repeats(n){};

	BTStatus decorate(uint32_t entityID)
	{
		for (int i = 0; i < this->num_repeats; i++)
		{
			lastReturn = this->children[0]->execute(entityID);
			if (lastReturn != BTStatus::Failure)
			{
				break;
			}
		}
		return lastReturn;
	}
};

class Breakpoint : public Decorator
{
   private:
	std::function<bool(uint32_t entityID)> conditionFunc;
   public:
	Breakpoint(std::function<bool(uint32_t)> condition = [](uint32_t ) -> bool {return true;}) :
	    Decorator(NodeType::Breakpoint, "Breakpoint", "break"),
	    conditionFunc(condition){};
	
	BTStatus decorate(uint32_t entityID) { 
        if (this->conditionFunc(entityID))
		{
            call_breakpoint();
        }
		lastReturn = this->children[0]->execute(entityID);
		return lastReturn;
	}
};


void Node::addCompositor(Compositor *compositor)
{
    children.push_back(compositor); compositor->parent = this;
}

void Node::addCompositors(std::vector<Compositor*> compositors)
{
    this->children.insert(this->children.end(), compositors.begin(), compositors.end());
    for(auto& child : compositors){child->parent = this;}
}

struct Fallback{
    Fallback(std::vector<Condition*>&& conditions,Compositor* compositor)
    {
        this->conditions = conditions;
        this->compositor = compositor;
    }
    std::vector<Condition*> conditions;
    Compositor* compositor;
};

void Node::addFallback( Fallback&& fallback)
{
    for(auto conditional : fallback.conditions)
    {
        this->leafs.push_back(conditional);
        this->children.push_back(conditional);
        conditional->parent = this;
    }
              
    fallback.compositor->parent = this;
    this->children.push_back(fallback.compositor);
}
void Node::addFallbacks(std::vector<Fallback> fallbacks)
{
    for(auto fallback : fallbacks )
    {
        addFallback(std::move(fallback));
    }
}

void Node::addChildren(std::vector<Node*> children)
{
    this->children.insert(this->children.end(), children.begin(), children.end());
    for(auto child : children){child->parent = this;}
}

void Node::addLeafs(std::vector<Leaf*> tasks)
{
    this->leafs.insert(this->leafs.end(), tasks.begin(), tasks.end());
    for(auto task : tasks){task->parent = this;}

    this->children.insert(this->children.end(), tasks.begin(), tasks.end());
}
void Node::addLeaf(Leaf* task)
{
    leafs.push_back(task);
    task->parent = this;        
    this->children.push_back(task);
}

void Node::addDecorator(Decorator* decorator)
{
    decorators.push_back(decorator);
    decorator->parent = this;        
    this->children.push_back(decorator);
}
