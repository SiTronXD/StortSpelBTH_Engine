#include "AIHandler.hpp"
#include "imgui.h"

void AIHandler::drawImgui(){

    if(FSMimguiLambdas.size() <= 0 ){return;}
    static bool blackboard_Entities_open = true; 
    ImGui::Begin("BlackBoard");
    float width = ImGui::GetWindowWidth();
    if(ImGui::BeginTabBar("BlackboardTabs", ImGuiTabBarFlags_None)){
        
        if (ImGui::BeginTabItem("Entities"))
        {   
            
            uint32_t id = 0;

            static std::string entity_label;
            static uint32_t selected_entity = 0;                
            static std::function<void(FSM*, uint32_t)> selected_entity_func;                
            static FSM* selected_fsm = nullptr;
            static bool useFirstAsDefault = true;    
            
            ImGui::SetNextItemOpen(blackboard_Entities_open, ImGuiCond_FirstUseEver);
            ImGui::BeginChild("entity_picker", ImVec2(100, 0), false, ImGuiWindowFlags_AlwaysAutoResize);
            
            for(auto fsmImguiLambda : this->FSMimguiLambdas)
            {
                for(auto entityId : this->FSMsEntities[fsmImguiLambda.first])
                {
                    
                    entity_label = "Entity["+std::to_string(entityId)+"]";
                    if(ImGui::Selectable(entity_label.c_str(), selected_entity == entityId) || useFirstAsDefault)
                    { 
                        selected_entity = entityId;
                        
                        selected_entity_func = fsmImguiLambda.second;
                        selected_fsm = fsmImguiLambda.first;
                        useFirstAsDefault = false; 
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