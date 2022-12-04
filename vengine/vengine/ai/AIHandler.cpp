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
    ImGui::Checkbox("Disable all AI", &this->disableAI);
    
    if(ImGui::BeginTabBar("BlackboardTabs", ImGuiTabBarFlags_None)){
                
        for(auto fsm : this->FSMs)
        {
            const std::string&  fsmName = fsm.first;
            FSM*          fsmPtr = fsm.second;
            if (ImGui::BeginTabItem(fsmName.c_str()))
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
                            
                auto fsmImguiLambda = this->FSMimguiLambdas[fsmPtr];
                
                for(auto entityId : this->FSMsEntities[fsmPtr])
                {
                    
                    entity_label = "Entity["+std::to_string(entityId)+"]";
                    if(ImGui::Selectable(entity_label.c_str(), selected_entity == entityId) )
                    { 
                        selected_entity = entityId;                        
                        selected_entity_func = fsmImguiLambda;
                        selected_fsm = fsmPtr;
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

        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
void AIHandler::update(float dt)
{
    this->dt = dt;
    if (currentScene != this->sh->getScene())
    {
        FSMimguiLambdas.clear();
        FSMsEntities.clear();
        currentScene = this->sh->getScene();
    }

    eventSystem.update();
    if (!disableAI)
    {
        updateEntityFSMs();
    }
    drawImgui();
    switchedScene = false;
}
void AIHandler::resetEventSystemLastReturn()
{
    this->eventSystem.resetEntityLastReturn();
}
void AIHandler::createAIEntity(uint32_t entityID, const std::string& fsmName)
{
    createAIEntity(entityID, this->FSMs[fsmName]);
}
void AIHandler::createAIEntity(uint32_t entityID, FSM* fsm)
{
    // Register entityID to a specific FSM (MovementFSM in this test)
    this->sh->getScene()->setComponent<FSMAgentComponent>(entityID, fsm);

    // Add Entity to IMGUI lambdas vector
    this->FSMsEntities[fsm].push_back(entityID);

    // Add required FSM and BT Components to entityID
    fsm->registerEntity(entityID);

    // Register this entity to all entity evenets of the FSM
    this->eventSystem.registerEntityToEvent(entityID, fsm);
}
void AIHandler::clean()
{

    this->eventSystem.clean();
    this->FSMimguiLambdas.clear();
    this->FSMsEntities.clear();

    for (auto p : this->FSMs)
    {
        if(p.second != nullptr)
        {
            p.second->clean();
            delete p.second;
        }
    }
    this->FSMs.clear();
}
void AIHandler::init(SceneHandler* sh)
{
    this->sh = sh;
    this->eventSystem.setSceneHandler(this->sh);
    
    this->clean();

    this->currentScene = this->sh->getScene();
    this->switchedScene = true;
}
void AIHandler::addImguiToFSM(const std::string& name, std::function<void(FSM* fsm, uint32_t)> imguiLambda)
{
    this->FSMimguiLambdas[this->FSMs[name]] = imguiLambda;
}
void AIHandler::updateEntityFSMs()
{
    auto& reg = this->sh->getScene()->getSceneReg();

    reg.view<FSMAgentComponent>(entt::exclude<Inactive>).each([&](const auto& entity, FSMAgentComponent& t) { t.execute(static_cast<int>(entity)); });
}
