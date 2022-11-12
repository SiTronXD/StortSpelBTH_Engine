#include "pch.h"
#include "Blackboard.hpp"

#include "glm/vec2.hpp"
#include <variant>
#include <typeindex>
#include <span>

template<typename T>
size_t getDataType() { return std::type_index(typeid(T)).hash_code();}
#define isType(dataHashCode, type) dataHashCode == getDataType<type>()
#define castData(T)  T& userdata = std::any_cast<T&>(data)
#define lamb [&userdata, key]() -> void

#ifdef IMGUI_DEBUG_AI_BT
std::vector<std::function<void()>> Blackboard::imguiLambdasEnv;
std::vector<std::vector<std::function<void()>>> Blackboard::imguiLambdasEntity;

void Blackboard::registerImguiComponentEnv(const std::string& key)
{            
    auto& data = Blackboard::getEnvironmentData(key);
    auto dataHashCode = data.type().hash_code();
    
    registerImguiComponent(key, dataHashCode, data, Blackboard::imguiLambdasEnv);            
}

void Blackboard::registerImguiComponentEntity(const std::string& key, const uint32_t entityId)
{            
    auto& data = Blackboard::getEntityData(key, entityId);
    auto dataHashCode = data.type().hash_code();

    static std::unordered_map<uint32_t, uint32_t> entityidToIndex;

     if(entityidToIndex.count(entityId) == 0)
     {
        entityidToIndex.insert({entityId,entityidToIndex.size()});
        Blackboard::imguiLambdasEntity.resize(entityidToIndex.size());
    }

    
    
    registerImguiComponent(key+"["+std::to_string(entityId)+"]", dataHashCode, data, Blackboard::imguiLambdasEntity[entityidToIndex[entityId]]);
}

void Blackboard::registerImguiComponent(
    const std::string& key, 
    size_t dataHashCode, 
    std::any& data,
    std::vector<std::function<void()>>& lambdVec)
{
    using BB = Blackboard;
    if (isType(dataHashCode, int)) 
    {        
        castData(int);        
        lambdVec.push_back(                        
            lamb{
                ImGui::InputInt(key.c_str(), &userdata);                
            }
        );
    }        
    else if (isType(dataHashCode, bool))
    {  
        castData(bool);
        lambdVec.push_back(                        
            lamb{
                ImGui::Checkbox(key.c_str(), &userdata);   
            }
        );

    }
    else if (isType(dataHashCode, double))
    {
        castData(double);
        lambdVec.push_back(                        
            lamb{
                ImGui::InputDouble(key.c_str(), &userdata);                
            }
        );

    }
    else if (isType(dataHashCode, float))
    {
        castData(float);
        lambdVec.push_back(                        
            lamb{
                ImGui::InputFloat(key.c_str(), &userdata);                
            }
        );
    }
    else if (isType(dataHashCode, std::string))
    {
        castData(std::string);
        lambdVec.push_back(                        
            lamb{
                ImGui::Text(key.c_str(), userdata.c_str());
            }
        );
    }
    else if (isType(dataHashCode, char*))
    {
        castData(char*);
        lambdVec.push_back(                        
            lamb{
                ImGui::Text(key.c_str(), &userdata);
            }
        );
    }
    else if (isType(dataHashCode, glm::vec3))
    {
        castData(glm::vec3);
        float* temp[3] = 
        {
            &userdata.x,
            &userdata.y,
            &userdata.z
        };        
        lambdVec.push_back(                        
            [temp, key]() -> void{
                ImGui::InputFloat3(key.c_str(), *temp);
            }
        );
    }
    else if (isType(dataHashCode, glm::vec2))
    {
        castData(glm::vec2);
        float* temp[2] = 
        {
            &userdata.x,
            &userdata.y
        };        
        lambdVec.push_back(                        
            [temp, key]() -> void{
                ImGui::InputFloat3(key.c_str(), *temp);
            }
        );

    }
    else
    {
        //TODO: Reconsider Assert here, maybe warning is enough? 
        std::cout << "Warning! Given key [" << key <<  "] cant be registered as a imgui component\n";
        //assert(false && "Missing type in registerImguiComponent func! ");
    }
}


void Blackboard::drawImgui()
{
    static bool blackboard_open = true;
    static bool blackboard_Entities_open = true;
    ImGui::Begin("BlackBoard", &blackboard_open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize );
    float width = ImGui::GetWindowWidth();
    if(ImGui::BeginTabBar("BlackboardTabs", ImGuiTabBarFlags_None)){
        
        if (ImGui::BeginTabItem("Enviroment"))
        {   
            auto middle = Blackboard::imguiLambdasEnv.begin()+Blackboard::imguiLambdasEnv.size()/2.f;
            //for(auto lambda : Blackboard::imguiLambdasEnv)
            std::span<std::function<void()>> part_1 =
                {Blackboard::imguiLambdasEnv.begin(),middle};
            std::span<std::function<void()>> part_2 =
                {middle, Blackboard::imguiLambdasEnv.end()};

            ImGui::BeginChild("Enviroment_left", ImVec2(width/2.f, 0), false , ImGuiWindowFlags_AlwaysAutoResize);
            for(auto lambda : part_1)
            {
                lambda();
            }

            ImGui::EndChild();
            ImGui::SameLine();     

            ImGui::BeginChild("Enviroment_right", ImVec2(0, 0), false);
            for(auto lambda : part_2)
            {
                lambda();
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Entities"))
        {   
    
            uint32_t id = 0;

            static std::string entity_label;
            static uint32_t selected_entity = 0;                
            
            ImGui::SetNextItemOpen(blackboard_Entities_open, ImGuiCond_FirstUseEver);
            ImGui::BeginChild("entity_picker", ImVec2(100, 0), false, ImGuiWindowFlags_AlwaysAutoResize);
            
            for(auto entity : Blackboard::imguiLambdasEntity)
            {                   
                entity_label = "Entity["+std::to_string(id)+"]";
                if(ImGui::Selectable(entity_label.c_str(), selected_entity == id))
                { selected_entity = id;}

                id++;       
            }
            ImGui::EndChild();
            ImGui::SameLine();        
            ImGui::BeginGroup();
            ImGui::BeginChild("Entity_view",ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysAutoResize); 
            ImGui::Text("Entity[%d]", selected_entity);
            ImGui::Separator();
            if (ImGui::BeginTabBar("Tabs", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Data"))
                {                
                    auto entity = Blackboard::imguiLambdasEntity[selected_entity];
                    for(auto lambda : entity)
                    {
                        lambda();
                    }            
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
            ImGui::EndChild();
            ImGui::EndGroup();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    


    ImGui::End();
}
#endif 

#undef TypeOf
#undef isType
#undef lamb