#include "game_object.h"
#include "../kv.h"

GameObject::GameObject(entt::entity e) : TaggedObject(e)
{

}

TransformComponent& GameObject::Transform()
{
	return GetComponent<TransformComponent>();
}

void GameObject::SetHighlightState(bool state)
{
    GetPropertyStorage<GameObjectProperties>().m_HighlightState = state;
}

void GameObject::SetHighlightColor(Color color)
{
    GetPropertyStorage<GameObjectProperties>().m_HighlightColor = color;
}

Color GameObject::GetHighlightColor()
{
    return GetPropertyStorage<GameObjectProperties>().m_HighlightColor;
}



bool GameObject::IsActive()
{
	return GetPropertyStorage<GameObjectProperties>().m_Active;
}

bool GameObject::GetHighlightState()
{
	return GetPropertyStorage<GameObjectProperties>().m_HighlightState;
}


void GameObject::SetActiveState(bool state)
{
    GetPropertyStorage<GameObjectProperties>().m_Active = state;
}


void GameObject::ShowProperties()
{
    static std::vector<std::string> idsToErase;
    static ImVec2 tableSize = ImVec2(0, 0);

    ImGui::SetCursorPosX(0);
    if (ImGui::BeginTable(GuiLayer::GetImGuiID(&GetPropertyStorage<GameObjectProperties>()).c_str(),1,ImGuiTableFlags_Borders | ImGuiTableFlags_ContextMenuInBody,ImVec2(ImGui::GetWindowSize().x,0),10)) {
        //ImGui::Text(("Handle = " + std::to_string((size_t)m_MasterHandle)).c_str());

        

        for (auto& name : Properties().GetComponentsNames()) {
            auto* comp = GetComponentByName(name);

            if (!comp->IsVisibleInEditor()) {
                continue;
            }

            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2, 0.2, 0.2, 1));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.2, 0.2, 0.2, 1));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 1));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 1));
            //ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,10);
            //ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,10);

            

            
            ImGui::TableNextColumn();

            
            
     
            
            ImGui::SetCursorPosX(2);
            ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
            bool selected;
            
            GuiLayer::SetupStaticTreeNodeStyle([&]() {
                selected = ImGui::TreeNodeEx((Registry::GetComponentDisplayName(name)).c_str(), ImGuiTreeNodeFlags_Selected |ImGuiTreeNodeFlags_SpanFullWidth );
            });
            
                
            if (selected) {


                ImGui::SameLine();



                if (comp->CanBeDisabled()) {
                    //ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + 10, ImGui::GetCursorPos().y - 2));
                    //ImGui::Checkbox("##", );
                    //ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x - 10, ImGui::GetCursorPos().y + 2));
                }
                ImGui::SameLine();

                
                ImGui::Spacing();
                
                comp->ShowProperties();
                ImGui::TreePop();
                

                ImGui::InvisibleButton("##", ImVec2(1, 5));
            }

            
            
            
            ImGui::PopStyleColor(4);
            //ImGui::PopStyleVar(1);


        }
        
        ImGui::EndTable();
    }
    if (idsToErase.size() > 0) {
        for (auto& id : idsToErase) {
            EraseComponentByName(id);
        }
        idsToErase.clear();
    }
}




