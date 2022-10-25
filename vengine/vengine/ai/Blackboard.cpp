#include "Blackboard.hpp"
#include <iostream>

std::unordered_map<std::string, BlackboardData> Blackboard::environmentData;
std::unordered_map<uint32_t, EntityBbData> Blackboard::entityData;

void Blackboard::addEnvironmentData(const std::string& key, std::any value)
{

    if(Blackboard::environmentData.count(key) <= 0 )
    {
        // TODO: Handle case: Key already in use
        Blackboard::environmentData.insert({key, {value.type().name(),value}});
        
// IMGUI STUFF! Does not work in VENGINE...
#ifdef IMGUI_DEBUG_AI_BT
        Blackboard::registerImguiComponentEnv(key);
#endif
    }
    else
    {
        std::cout << "addEnviromentData: Key ["<< key <<"] already exists!\n";
        assert(false && "addEnviromentData function failed");
    }
}

void Blackboard::addEntityData(const std::string& key, const uint32_t entityId, std::any value)
{
    EntityBbData* entDta = nullptr;

    if(Blackboard::entityData.count(entityId) > 0 )
    {
        entDta = &Blackboard::entityData.find(entityId)->second;
    }
    else 
    {        
        Blackboard::entityData.insert({entityId, EntityBbData{}} );
        entDta = &Blackboard::entityData.find(entityId)->second;
    }

    if(entDta->data.count(key) <= 0)
    {
        entDta->data.insert({key, {value.type().name(), value}});

// IMGUI STUFF! Does not work in VENGINE...
#ifdef IMGUI_DEBUG_AI_BT
        Blackboard::registerImguiComponentEntity(key, entityId);
#endif
    }
    else
    {
        // TODO: Change this, if data already exists, then update it! (?? maybe)
        std::cout << "addEntityData: Key ["<< key <<"] already exists!\n";
        assert(false && "addEntityData function failed");
    }
}

//TODO: Define
void Blackboard::removeEnvironmentData(const std::string& key)
{
    //return Blackboard::environmentData.find(key)->second.value;
}

void Blackboard::removeEntityData(const std::string& key, const uint32_t entity)
{
    //return Blackboard::entityData.find(entity)->second.data.find(key)->second.value;
}

std::any& Blackboard::getEnvironmentData(const std::string& key)
{
    return Blackboard::environmentData.find(key)->second.value;
}

std::any& Blackboard::getEntityData(const std::string& key, const uint32_t entity)
{
    return Blackboard::entityData.find(entity)->second.data.find(key)->second.value;
}
