#include "pch.h"
#include "AIHandler.hpp"
#include "imgui.h"

void AIHandler::drawImgui(){

    // TODO: Improve this code... 
    // If any entities exists, then we can run without problem... 
    if(this->FSMimguiLambdas.size() <= 0 ){return;}
    bool noEntities = true; 
    for( auto& fsm : this->FSMsEntities)
    {if(fsm.second.size() > 0) {noEntities = false; break;}}
    if(noEntities) {return; }
    
    static bool blackboard_Entities_open = true; 
    ImGui::Begin("BlackBoard");
    float width = ImGui::GetWindowWidth();
    if(ImGui::BeginTabBar("BlackboardTabs", ImGuiTabBarFlags_None)){
        
        if (ImGui::BeginTabItem("Entities"))
        {   

            static std::string entity_label;
            static uint32_t selected_entity = 0;                
            static std::function<void(FSM*, uint32_t)> selected_entity_func;                
            static FSM* selected_fsm = nullptr;
            
            ImGui::SetNextItemOpen(blackboard_Entities_open, ImGuiCond_FirstUseEver);
            ImGui::BeginChild("entity_picker", ImVec2(100, 0), false, ImGuiWindowFlags_AlwaysAutoResize);

            // Use first as default! 
            if(this->switchedScene)
            {
                selected_entity      = this->FSMsEntities[this->FSMimguiLambdas.begin()->first][0];                        
                selected_entity_func = this->FSMimguiLambdas.begin()->second;
                selected_fsm         = this->FSMimguiLambdas.begin()->first;
            }
                        
            for(auto fsmImguiLambda : this->FSMimguiLambdas)
            {
                for(auto entityId : this->FSMsEntities[fsmImguiLambda.first])
                {
                    
                    entity_label = "Entity["+std::to_string(entityId)+"]";
                    if(ImGui::Selectable(entity_label.c_str(), selected_entity == entityId) )
                    { 
                        selected_entity = entityId;                        
                        selected_entity_func = fsmImguiLambda.second;
                        selected_fsm = fsmImguiLambda.first;
                    }  
                }
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
                    selected_entity_func(selected_fsm,selected_entity);                    
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