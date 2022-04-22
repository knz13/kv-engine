#include "properties_view.h"
#include "../kv.h"

void GuiLayer::PropertiesView::Update(Window& win) {
    GuiLayer::SetupWindowStyle("Properties",[&](ImGuiWindowFlags flags){
        ImGui::Begin("Properties",0,flags );
    });
    static bool randomAddress = false;

    

    if (GameView::AnyObjectSelected()) {
        GameObject selected(GameView::AnyObjectSelected().objectID);
        selected.ShowProperties();

        ImGui::SetCursorPosX(ImGui::GetWindowSize().x / 2 - ImGui::CalcTextSize("Add Component").x / 2);
        
        GuiLayer::SetupStaticButtonStyle([]() {
            ImGui::Button("Add Component");
        });
        if (ImGui::BeginPopupContextItem(GuiLayer::GetImGuiID(&randomAddress).c_str(), ImGuiPopupFlags_MouseButtonLeft)) {

            for (auto& componentName : GameObject::GetRegisteredComponents()) {
                if (!selected.HasComponent(componentName)) {
                    if (ImGui::MenuItem(componentName.c_str())) {
                        selected.AddComponentByName(componentName);
                    }
                }
            }
            
                

            ImGui::EndPopup();
        }
        

    }

    ImGui::End();
}
