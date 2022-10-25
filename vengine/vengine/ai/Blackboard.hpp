#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <any>
#include <functional>
#include <iostream>
#include <glm/vec3.hpp>

struct BlackboardData{
    std::string type;
    std::any value;
};
struct EntityBbData{
    std::unordered_map<std::string, BlackboardData> data;
};
class Blackboard
{
private:
    static std::unordered_map<std::string, BlackboardData> environmentData;
    static std::unordered_map<uint32_t, EntityBbData> entityData;
    Blackboard() = default;
public:

    static void addEnvironmentData(const std::string& key, std::any value);
    static void addEntityData(const std::string& key, const uint32_t entity, std::any value);
    
    template<typename T>
    static void setEnvironmentData(const std::string& key, T value);
    template<typename T>
    static void setEntityData(const std::string& key, const uint32_t entity, T value);

    static std::any& getEnvironmentData(const std::string& key);
    static std::any& getEntityData(const std::string& key, const uint32_t entity);

    //TODO: Define
    static void removeEnvironmentData(const std::string& key);
    static void removeEntityData(const std::string& key, const uint32_t entity);

    template<typename T>
    static T& getEnv(const std::string& key);
    template<typename T>
    static T& getEnt(const std::string& key, const uint32_t entityId);


// Debug Related
private:
// IMGUI STUFF! Does not work in VENGINE...
#ifdef IMGUI_DEBUG_AI_BT
    static std::vector<std::function<void()>> imguiLambdasEnv;
    static std::vector<std::vector<std::function<void()>>> imguiLambdasEntity;
    

    static void registerImguiComponentEnv(const std::string& key);
    static void registerImguiComponentEntity(const std::string& key, const uint32_t entityId);
    static void registerImguiComponent(const std::string& key, size_t dataHashCode, std::any& data, std::vector<std::function<void()>>& lambdVec);
#endif 
public:     
    //static void drawImgui();
};

template<typename T>
T& Blackboard::getEnv(const std::string& key)
{
    if(Blackboard::environmentData.count(key) <= 0)    
    {
        std::cout << "getEnv: Key ["<< key <<"] does not exists!\n";
        assert(false && "getEnv function failed");
    }

    return std::any_cast<T&>(Blackboard::getEnvironmentData(key));
}

template<typename T>
T& Blackboard::getEnt(const std::string& key, const uint32_t entityId)
{
    if(Blackboard::entityData.count(entityId) <= 0)    
    {
        std::cout << "getEnv: Entity ID ["<< entityId <<"] does not exists!\n";
        assert(false && "getEnv function failed");
    }
    if(Blackboard::entityData.find(entityId)->second.data.count(key) <= 0)    
    {
        std::cout << "getEnv: Key ["<< key <<"] for Entity [" << entityId <<"] does not exists!\n";
        assert(false && "getEnv function failed");
    }
        
    return std::any_cast<T&>(Blackboard::getEntityData(key,entityId));
}


template<typename T>
void Blackboard::setEnvironmentData(const std::string& key, T value)
{
    if(Blackboard::environmentData.count(key) > 0 )
    {                           
        T& ref = std::any_cast<T&>(Blackboard::getEnvironmentData(key));
        ref = value;
    }
    else
    {
        std::cout << "setEnvironmentData: Key ["<< key <<"] was not found!\n";
        assert(false && "setEnvironmentData function failed");
    }    
}

template<typename T>
void Blackboard::setEntityData(const std::string& key, const uint32_t entityId, T value)
{
    EntityBbData* entDta = nullptr;

    if(Blackboard::entityData.count(entityId) > 0 )
    {
        entDta = &Blackboard::entityData.find(entityId)->second;
    }
    else 
    {        
        std::cout << "setEntityData: Entity with ID ["<< entityId <<"] was not found!\n";
        assert(false && "setEntityData function failed");
    }

    if(entDta->data.count(key) > 0)
    {
        T& ref = std::any_cast<T&>(Blackboard::getEntityData(key,entityId));
        ref = value;
    }
    else
    {       
        std::cout << "setEntityData: Key ["<< key <<"] for Entity with ID ["<< entityId <<"] was not found!\n";
        assert(false && "setEntityData function failed");
    }
    
}