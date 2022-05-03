#include "window.h"
#include "kv.h"


std::map<GLFWwindow*,Window*> Window::m_CurrentWindows;
EventLauncher<void(Window&)> Window::m_StartWindowFuncs;
Window* Window::m_MainWindow = nullptr;

Window::Window(WindowCreationProperties prop) : m_Properties(prop) {

    Window::m_MainWindow = this;
    if(glfwInit() != GLFW_TRUE){
        DEBUG_ERROR("GLFW was not initiated!");
        return;
    }

    if(prop.windowFlags != WindowFlag::None){
        if(prop.windowFlags & WindowFlag::OpenGLDebugContext){
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,GLFW_TRUE);
        }
        if(prop.windowFlags & WindowFlag::InitiallyMinimized){
            glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
        }
        if(prop.windowFlags & WindowFlag::NotDecorated){
            glfwWindowHint(GLFW_DECORATED,GLFW_FALSE);
        }
        if(prop.windowFlags & WindowFlag::NotResizeable){
            glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);
        }
        if(prop.windowFlags & WindowFlag::NotFocused){
            glfwWindowHint(GLFW_FOCUSED,GLFW_FALSE);
        }
        if(prop.windowFlags & WindowFlag::FocusOnShow){
            glfwWindowHint(GLFW_FOCUS_ON_SHOW,GLFW_TRUE);
        }
        
        if((prop.openGLVersionMajor == 3 && prop.openGLVersionMinor > 2) || prop.openGLVersionMajor > 3){
            if(prop.windowFlags & WindowFlag::CoreProfile){
                glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
            }
            else {
                glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_COMPAT_PROFILE);
            }
        }
    }

    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,prop.openGLVersionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,prop.openGLVersionMinor);

    if(prop.windowFlags & WindowFlag::FullScreen){
        this->m_ContextPointer = glfwCreateWindow(prop.width,prop.height,prop.title.c_str(),glfwGetPrimaryMonitor(),NULL);
    }
    else {
        this->m_ContextPointer = glfwCreateWindow(prop.width,prop.height,prop.title.c_str(),NULL,NULL);
    }

    Window::m_CurrentWindows[this->m_ContextPointer] = this;
    

    glfwMakeContextCurrent(this->m_ContextPointer);

    glewExperimental = true;
    if(glewInit() != GLEW_OK){
        DEBUG_ERROR("Glew wasn't initiated!");
        return;
    }


    

    GameObject mainCamera = ObjectPropertyRegister::CreateNew<GameObject>("Main Camera");
    mainCamera.AddComponent<Camera>();
    mainCamera.Transform().SetPosition(0, 5, 2);

    mainCamera.GetComponent<Camera>().SetLookAt(0, 0, 0);

    m_MainCamera = mainCamera.ID();

    

    
    GL_CALL(glEnable(GL_PROGRAM_POINT_SIZE));
    GL_CALL(glEnable(GL_STENCIL_TEST));
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glEnable(GL_CULL_FACE));
    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    glfwSetKeyCallback(this->GetContextPointer(),[](GLFWwindow* ptr,int key, int scancode, int action, int mods){
        Window& win = *Window::GetWindow(ptr);
        KeyEventProperties prop;
        prop.key = key;
        prop.scancode = scancode;
        prop.action = action;
        prop.mods = mods;
        win.m_KeyEventFuncs.EmitEvent(win,prop);
    });

    glfwSetCursorPosCallback(this->GetContextPointer(),[](GLFWwindow* window, double xpos, double ypos){
        Window& win = *Window::GetWindow(window);
        MouseEventProperties prop;
        prop.position = glm::vec2(xpos,ypos);
        win.m_MouseMovedFuncs.EmitEvent(win,prop);
    });

    glfwSetCursorEnterCallback(this->GetContextPointer(),[](GLFWwindow* window,int entered){
        double xpos,ypos;
        glfwGetCursorPos(window,&xpos,&ypos);
        MouseEventProperties prop;
        prop.position = glm::vec2(xpos,ypos);
        Window& win = *Window::GetWindow(window);
        if(entered){
            win.m_MouseEnteredWindowFuncs.EmitEvent(win,prop);
        }
        else {
            win.m_MouseLeftWindowFuncs.EmitEvent(win,prop);
        }
    });

    glfwSetMouseButtonCallback(this->GetContextPointer(),[](GLFWwindow* window, int button, int action, int mods){
        double xpos,ypos;
        glfwGetCursorPos(window,&xpos,&ypos);
        Window& win = *Window::GetWindow(window);
        MouseButtonEventProperties prop;
        prop.button = button;
        prop.action = action;
        prop.mods = mods;
        prop.position = glm::vec2(xpos,ypos);
        win.m_MouseButtonFuncs.EmitEvent(win,prop);
    });

    glfwSetScrollCallback(this->GetContextPointer(),[](GLFWwindow* window, double xoffset, double yoffset){
        Window& win = *Window::GetWindow(window);
        MouseScrollEventProperties prop;
        prop.offset = glm::vec2(xoffset,yoffset);
        win.m_MouseScrollFuncs.EmitEvent(win,prop);
    });

    glfwSetFramebufferSizeCallback(this->GetContextPointer(),[](GLFWwindow* window, int width, int height){
        Window& win = *Window::GetWindow(window);
        WindowResizedEventProperties prop;
        prop.width = width;
        prop.height = height;
        win.m_WindowResizedEventFuncs.EmitEvent(win,prop);
    });

    glfwSetWindowCloseCallback(this->GetContextPointer(),[](GLFWwindow* window){
        Window& win = *Window::GetWindow(window);
        win.m_IsOpen = false;
        
    });

    glfwSetWindowFocusCallback(this->GetContextPointer(), [](GLFWwindow* window, int focused) {
        bool hasFocus = focused == GLFW_TRUE ? true : false;

        Window& win = *Window::GetWindow(window);
        win.m_FocusEventFuncs.EmitEvent(win,hasFocus);

        
    });

    this->Events().ResizedEvent().Connect([](Window& win,WindowResizedEventProperties prop){
        win.m_Properties.width = prop.width;
        win.m_Properties.height = prop.height;
        

    });
    


    //handle creation events

    Window::m_StartWindowFuncs.EmitEvent(*this);

    

}

Window::~Window() {
    
    
    m_CreatedShaders.clear();
    m_CreatedVertexArrays.clear();

    

    Window::m_CurrentWindows.erase(m_ContextPointer);

    glfwDestroyWindow(m_ContextPointer);
}

bool Window::IsOpen() {
    return !glfwWindowShouldClose(m_ContextPointer);
}

GLFWwindow* Window::GetContextPointer() {
    return m_ContextPointer;
}

void Window::EndDrawState() {
    m_PostDrawingLoopFuncs.EmitEvent(*this);
    
    
    glfwSwapBuffers(m_ContextPointer);

}

void Window::BeginDrawState() {
    m_PreDrawingLoopFuncs.EmitEvent(*this);

    glfwPollEvents();

    glm::vec3 color = m_ClearColor.Normalized();
    GL_CALL(glClearColor(color.x,color.y,color.z,1.0f));
    GL_CALL(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
    GL_CALL(glStencilMask(0xFF));
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}



void Window::SetClearColor(Color color) {
    m_ClearColor = color;
}

const WindowCreationProperties& Window::Properties() const {
    return m_Properties;
}


void Window::SetCamera(GameObject obj) {
    if(obj.HasComponent<Camera>()){
        m_MainCamera = obj.ID();
    }
}



void Window::SetViewPort(int x, int y, int width, int height)
{
    if (GetCurrentCamera()) {
        m_Properties.width = width - x;
        m_Properties.height = height - y;
        //glm::vec4 viewport = GetCurrentCamera().GetAsObject().GetComponent<Camera>().GetViewPort();
        //GL_CALL(glViewport(x,y,width,height));
    }
}



TemplatedObjectHandle<GameObject> Window::GetCurrentCamera() {
    return TemplatedObjectHandle<GameObject>(m_MainCamera);
}



WindowEvents Window::Events() {
    return WindowEvents(*this);
}

void Window::DrawingLoop() {
    static float currentTime=0,oldTime=0;

    while(IsOpen()){

        glfwMakeContextCurrent(m_ContextPointer);

        
        
        BeginDrawState();
        
        currentTime = glfwGetTime();
        m_DeltaTime = static_cast<float>(currentTime - oldTime);

        oldTime = currentTime;

        GameObject::ForEach([&](GameObject obj) {
            for (auto& name : obj.Properties().GetComponentsNames()) {
                obj.GetComponentByName(name)->Update(m_DeltaTime);
            }
        });
            
           

        

        auto view = Registry::Get().view<Camera>();
        for (auto entity : view) {
            Camera& camera = view.get<Camera>(entity);
            if (!GameObject(entity).IsActive()) {
                continue;
            }
            if (!camera.IsEnabled()) {
                continue;
            }
            if (!camera.HasRenderTarget()) {
                continue;
            }

            camera.Render();
        }

        EndDrawState();

        PostDrawOperations();
        
    }

    


    ObjectPropertyRegister::Each([](Object object){
        ObjectPropertyRegister::DeleteObject(object);
    });


    ObjectPropertyRegister::ClearDeletingQueue();
    Registry::Get().clear();
}



Window* Window::GetWindow(GLFWwindow* win) {
    if(Window::m_CurrentWindows.find(win) != Window::m_CurrentWindows.end()){
        return Window::m_CurrentWindows[win];
    }
    return nullptr;
}

VertexArray& WindowCreators::NewVertexArray() {
    VertexArray& vertex = *m_Master.m_CreatedVertexArrays.emplace_back(std::make_unique<VertexArray>()).get();
    return vertex;
}

Shader& WindowCreators::CachedShader(std::string shaderRelativePath, bool* loadResult) {
    if(m_Master.m_CreatedShaders.find(shaderRelativePath) != m_Master.m_CreatedShaders.end()){
        Shader& shader = *m_Master.m_CreatedShaders[shaderRelativePath].get();
        if(loadResult){
            *loadResult=true;
        }
        return shader;
    }
    
    m_Master.m_CreatedShaders[shaderRelativePath] = std::make_unique<Shader>();
    Shader& shader = *m_Master.m_CreatedShaders[shaderRelativePath].get();

    if(!std::filesystem::exists(shaderRelativePath)){
        LOG("Could not load shader at path " + std::filesystem::absolute(shaderRelativePath).string() + " passing empty shader");
        if(loadResult){
            *loadResult=false;
        }
        return shader;
    }

    std::vector<std::pair<ShaderType,std::string>> sources;
    for(auto file : std::filesystem::directory_iterator(shaderRelativePath)){
        std::string fileName = file.path().filename().string();

        if(fileName.ends_with("vert")){
            std::string source = LoadFileContents(std::filesystem::absolute(shaderRelativePath + "/" + fileName).string());
            sources.push_back(std::make_pair(ShaderType::Vertex,source));
        }
        if(fileName.ends_with("frag")){
            std::string source = LoadFileContents(std::filesystem::absolute(shaderRelativePath + "/" + fileName).string());
            sources.push_back(std::make_pair(ShaderType::Fragment,source));
        }
    }
    

    ShaderCreationProperties prop = m_Master.m_CreatedShaders[shaderRelativePath].get()->CreateNew();
    for(auto& [type,source] : sources){
        prop.AddShader(type,source);
    }

    if(!prop.Generate()){
        if(loadResult){
            *loadResult=false;
        }
        return shader;
    }

    if(loadResult){
        *loadResult=true;
    }
    return shader;

}

WindowCreators Window::Create() {
    return WindowCreators(*this);
}





FunctionSink<void(Window&,MouseEventProperties)> WindowEvents::MouseLeftWindowEvent() {
    return FunctionSink<void(Window&,MouseEventProperties)>(m_Master.m_MouseLeftWindowFuncs);
}



FunctionSink<void(Window&,MouseEventProperties)> WindowEvents::MouseEnteredWindowEvent() {
    return FunctionSink<void(Window&,MouseEventProperties)>(m_Master.m_MouseEnteredWindowFuncs);
}

FunctionSink<void(Window&,MouseButtonEventProperties)> WindowEvents::MouseButtonEvent() {
    return FunctionSink<void(Window&,MouseButtonEventProperties)>(m_Master.m_MouseButtonFuncs);
}

FunctionSink<void(Window&, bool)> WindowEvents::FocusEvent()
{
    return FunctionSink<void(Window&, bool)>(m_Master.m_FocusEventFuncs);
}

FunctionSink<void(Window&,MouseScrollEventProperties)> WindowEvents::MouseScrollEvent() {
    return FunctionSink<void(Window&,MouseScrollEventProperties)>(m_Master.m_MouseScrollFuncs);
}

FunctionSink<void(Window&,WindowResizedEventProperties)> WindowEvents::ResizedEvent() {
    return FunctionSink<void(Window&,WindowResizedEventProperties)>(m_Master.m_WindowResizedEventFuncs);
}

FunctionSink<void(Window&,KeyEventProperties)> WindowEvents::KeyEvent() {
    return FunctionSink<void(Window&,KeyEventProperties)>(m_Master.m_KeyEventFuncs);
}

FunctionSink<void(Window&, MouseEventProperties)> WindowEvents::MouseMovedEvent()
{
    return FunctionSink<void(Window&, MouseEventProperties)>(m_Master.m_MouseMovedFuncs);
}

FunctionSink<void(Window&)> Window::WindowCreationEvent() {
    return FunctionSink<void(Window&)>(Window::m_StartWindowFuncs);
}

FunctionSink<void(Window&)> WindowEvents::PreDrawingLoopEvent() {
    return FunctionSink<void(Window&)>(m_Master.m_PreDrawingLoopFuncs);
}

FunctionSink<void(Window&)> WindowEvents::PostDrawingLoopEvent() {
    return FunctionSink<void(Window&)>(m_Master.m_PostDrawingLoopFuncs);
}
FunctionSink<void(Window&)> WindowEvents::ClosingEvent() {
    return FunctionSink<void(Window&)>(m_Master.m_ClosingCallbackFuncs);
}


RayCastHit Window::RayCast(glm::vec2 screenPos) {
    return GuiLayer::GameView::RayCast(ImVec2(screenPos.x,screenPos.y));    
}

float Window::GetDeltaTime() {
    return m_DeltaTime;
}
Window& Window::GetCurrentWindow() {
    return *Window::m_MainWindow;
}

void Window::DisableCamera() {
    m_MainCamera = entt::null;
}

void Window::PostDrawOperations() {
    

    

    if(!m_IsOpen){
        m_ClosingCallbackFuncs.EmitEvent(*this);
    }

    ObjectPropertyRegister::ClearDeletingQueue();
}
