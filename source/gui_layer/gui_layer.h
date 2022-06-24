#pragma once 
#include "../global.h"
#include "explorer_view.h"
#include "game_view.h"
#include "properties_view.h"
#include "scene_hierarchy_view.h"
#include "console_view.h"
#include "../panels/panels.h"


class Window;

namespace GuiLayer {

    void Init();

    void AddUi(Window& win);
    

    std::string GetImGuiID(void* ptr);
    
    ecspp::ObjectHandle& AnyObjectSelected();

    template<typename T>
    void OpenWindow() {
        static_assert(std::is_base_of<Panel,T>::value,"Type is not derived from Panel!");
        if (PanelManager::RegisterPanel<T>()) {
            PanelManager::OpenPanel<T>();
        }
    }

    void PushConsoleText(ConsoleImportance importance,std::string text);
    void SetupWidgetStyle(std::function<void()> beginCommand);
    void SetupStaticTreeNodeStyle(std::function<void()> beginCommand);
    void SetupStaticButtonStyle(std::function<void()> beginCommand);
    void SetupWindowStyle(std::string windowName,std::function<void(ImGuiWindowFlags)> beginCommand);
    void SetupChildStyle(std::function<void()> command);
    bool LoadScene(std::string path);


    struct WindowIDs {
        
        static ImGuiID GetID(std::string name);
        static void CreateDock(std::string name, ImGuiID id);

        static inline std::unordered_map<std::string, ImGuiID> m_RegisteredIDs;
    };

    struct BaseSettings {
        static inline int FontScale = 1;
        static inline ImFont* MainFont = nullptr;
        static inline std::string LoadedScenePath = "";
    };

    struct BaseColors {
        static inline Color StaticWidgetBg = Color(60,60,60);
        static inline Color StaticWidgetActiveBg = Color(60,60,60);
        static inline Color StaticWidgetHoveredBg = Color(80, 80, 80);
        static inline Color WindowBg = Color(40,40,40);
    };

    


};

