#pragma once 
 #include "op_overload.hpp" 
#include "BehaviorTreeNodes.hpp"

class BTCreateHelper
{        
public:    
    struct Decorators
    {
        friend class BehaviorTree;
        private: 
        std::vector<Decorator*> registredDecorators;            
        uint32_t nrOfDecorators = 0;
        template<typename T, class ... Args>
        T* registerDecorator(Args...args)
        {
            nrOfDecorators++; 
            registredDecorators.push_back(new(__FILE__, __LINE__) T(args...));
            return (T*)registredDecorators.back(); //TODO can I return a casted pointer, or will it be local to this scope?
        }

        std::vector<Decorator*>& getDecorators(){return this->registredDecorators;}
        uint32_t getNrOfRegistredDecorators(){return this->nrOfDecorators;}
        public: 

        ForceSuccess*   forceSuccess(){return registerDecorator<ForceSuccess>();}; 
        ForceFailure*   forceFailure(){return registerDecorator<ForceFailure>();}; 
        Breakpoint*     breakpoint(std::function<bool(uint32_t)> condition = [](uint32_t ) -> bool {return true;})
        {return registerDecorator<Breakpoint>(condition);}; 
        Message*        message(const std::string& message){return registerDecorator<Message>(message);}; 
        Random*         random(){return registerDecorator<Random>();}; 
        Repeat*         repeat(int n = 1){return registerDecorator<Repeat>(n);}; 
        Invert*         invert(){return registerDecorator<Invert>();}; 
        Retry*          retry(int n = 1){return registerDecorator<Retry>(n);}; 
        Tag*            tag(const std::string& tag){return registerDecorator<Tag>(tag);}; 

    }decorator;
    Decorators&  d = decorator; //Shorthand for Decorators

    struct Compositors
    {
        private:
        friend class BehaviorTree;
        std::vector<Compositor*> registredCompositors;     
        uint32_t nrOfCompositors = 0;
        template<typename T, class ... Args>
        T* registerCompositor(Args...args)
        {
            nrOfCompositors++; 
            registredCompositors.push_back(new(__FILE__, __LINE__) T(args...));
            return (T*)registredCompositors.back(); //TODO can I return a casted pointer, or will it be local to this scope?
        }
        std::vector<Compositor*>& getCompositors(){return this->registredCompositors;}
        uint32_t getNrOfRegistredCompositors(){return this->nrOfCompositors;}
        public:

        Reference*      reference(){return registerCompositor<Reference>();} //TODO: Was Reference a Compositor Node or a Leaf Node? 
        Sequence*       sequence(){return  registerCompositor<Sequence>();} 
        Selector*       selector(){return  registerCompositor<Selector>();} 
        Parallel*       parallel(){return  registerCompositor<Parallel>();} 

    }compositor;
    Compositors& c = compositor; //Shorthand for Compositors

    struct Leafs
    {            
        private:
        friend class BehaviorTree;
        std::vector<Leaf*> registredLeafs;
        uint32_t nrOfLeafs = 0;
        template<typename T, class ... Args>
        T* registerLeaf(Args...args)
        {
            nrOfLeafs++; 
            registredLeafs.push_back(new(__FILE__, __LINE__) T(args...));
            return (T*)registredLeafs.back(); //TODO can I return a casted pointer, or will it be local to this scope?
        }
        std::vector<Leaf*>& getLeafs(){return this->registredLeafs;}
        uint32_t getNrOfRegistredLeafs(){return this->nrOfLeafs;}
        public: 

        Condition*      condition(const std::string& name, std::function<BTStatus(uint32_t)> conditional){return registerLeaf<Condition>(name, conditional);}; 
        Task*           task(const std::string& description, std::function<BTStatus(uint32_t)> process , std::function<void(uint32_t)> start = nullptr)
        {return registerLeaf<Task>(description,process,start);} //TODO: Task should only have 2 parameters? ... what is the hird used for?
        
    }leaf;
    Leafs& l = leaf; //Shorthand for leafs
    
};