#pragma once
#include "registry.h"
#include "object_properties_component.h"


template<typename T>
concept IsDerivedFromComponent = requires(T a){
    std::is_base_of<Component<T>,T>;
};

class TransformComponent;
class Object {
public:
    Object(entt::entity ent);
    ~Object();


    template<IsDerivedFromComponent T>
    bool HasComponent(){
        if(!this->Valid()){
            return false;
        }
        return Registry::Get().any_of<T>(m_EntityHandle);
    };

    bool HasComponent(entt::id_type type) {
        return Properties().m_AttachedComponentsProperties.find(type) != Properties().m_AttachedComponentsProperties.end();
    }

    bool HasComponent(std::string type){
        bool found = false;
        for(auto& [id,prop] : Properties().m_AttachedComponentsProperties){
            if(type == prop.m_ClassName){
                found = true;
            }
        }
        return found;
    }

    template<IsDerivedFromComponent T>
    T& GetComponent(){
        if(!this->HasComponent<T>()){
            T& comp = Registry::Get().emplace<T>(m_EntityHandle,m_EntityHandle);
            
            this->HandleComponent(comp);

            return comp;
        }
        else {
            return Registry::Get().get<T>(m_EntityHandle);
        }
    };

    TransformComponent& Transform();
    

    bool TryCopyComponent(std::string stringToHash,Object from){
        auto resolved = entt::resolve(entt::hashed_string(stringToHash.c_str()));
        
        if(resolved){
            entt::meta_any component = resolved.construct(*this);
            if(auto func = resolved.func(entt::hashed_string("Copy Component")) ; func){
                return func.invoke({},from,*this).operator bool();
            }
            else{
                return false;
            }
        }
        else{
            return false;
        }
    };

    bool TryAddComponent(std::string stringToHash){

        auto resolved = entt::resolve(entt::hashed_string(stringToHash.c_str()));
        if(resolved){
            entt::meta_any owner = resolved.construct(*this);
            return owner.operator bool();
        }
        else{
            return false;
        }
    }

    template<IsDerivedFromComponent T,typename ...Args>
    T& AddComponent(Args&&... args){
        if(!this->HasComponent<T>()){
            T& comp = Registry::Get().emplace<T>(m_EntityHandle,args...,m_EntityHandle);
            

            this->HandleComponent(comp);

            return comp;
        }
        else {
            return Registry::Get().get<T>(m_EntityHandle);
        }
    }

    const AttachedComponentProperties* GetComponentData(entt::id_type type){
        if(!HasComponent(type)){
            return nullptr;
        }

        return &Properties().m_AttachedComponentsProperties[type];
    } 




    template<IsDerivedFromComponent T>
    void EraseComponent(){
        if(!this->Valid()){
            
            return;
        }
        if(this->HasComponent<T>()){
            
            ObjectPropertiesComponent& properties = Properties();
            properties.EraseComponentProperties(entt::type_id<T>().index());
            
            Registry::Get().erase<T>(m_EntityHandle);
        }
    };

    template<IsDerivedFromComponent T>
    void EnableComponent() {
        if(!this->Valid()){
            return;
        }

        if(this->HasComponent<T>()){
            this->GetComponent<T>().SetActiveState(true);
        }
    }

    template<IsDerivedFromComponent T>
    void DisableComponent() {
        if(!this->Valid()){
            return;
        }

        if(this->HasComponent<T>()){
            this->GetComponent<T>().SetActiveState(false);
        }

    };

    bool Valid(){
        return Registry::Get().valid(m_EntityHandle);
    }

    
    
    ObjectPropertiesComponent& Properties() {
        return Registry::Get().get<ObjectPropertiesComponent>(m_EntityHandle);
    };

    const entt::entity& ID(){
        return Properties().m_MasterHandle;
    }

    template<IsDerivedFromComponent T>
    static bool CopyComponent(Object from,Object to){
        if(from.HasComponent<T>() && to.HasComponent<T>()){
            T& first = from.GetComponent<T>();
            T& second = to.GetComponent<T>();
            second = first;
            return true;
        }
        else{
            return false;
        }
    };

    static const std::vector<std::string>& GetRegisteredClasses(){
        return m_RegisteredComponents;
    }

private:
    template<IsDerivedFromComponent T>
    static void MakeComponentOmnipresent(){
        m_ClassesToAddEveryTime.push_back(Registry::GetClassName<T>());
    }

    template<IsDerivedFromComponent T>
    static void RegisterClassAsComponent(){
        entt::id_type hash = Registry::HashClassName<T>();
        entt::meta<T>().type(hash).ctor<&Object::GetComponent<T>,entt::as_ref_t>();
        entt::meta<T>().type(hash).func<&Object::CopyComponent<T>>(entt::hashed_string("Copy Component"));
        m_RegisteredComponents.push_back(Registry::GetClassName<T>());
    };
    template<IsDerivedFromComponent T>
    static entt::id_type HashComponent(){
        return entt::type_hash<T>().value();
    };

    template<typename T>
    void HandleComponent(T& comp){

        AttachedComponentProperties prop;
        prop.m_UpdateFunc = [&](float time){
            comp.Update(time);
        };

        prop.m_ShowPropertiesFunc = [&](){
            comp.ShowProperties();
        };


        comp.m_MyClassTypeID = HashComponent<T>();
        prop.m_ClassName = Registry::GetClassName<T>();
        prop.m_SizeInBytes = sizeof(T);
        prop.m_DisplayName = Registry::GetClassDisplayName<T>();
        prop.m_HideInEditor = &comp.m_ShouldHideInEditor;
        prop.m_ActiveState = &comp.m_BaseComponentActiveState;
        ObjectPropertiesComponent& properties = Properties();
        properties.HandleComponentProperties(HashComponent<T>(),prop);
    }

    entt::entity m_EntityHandle;
    static inline std::vector<std::string> m_ClassesToAddEveryTime;
    static inline std::vector<std::string> m_RegisteredComponents;


    friend class Registry;
    template<typename>
    friend class AddToEveryObject;

    template<typename>
    friend class AddWhenCalled;

    template<typename,typename>
    friend class Component;
};


class ObjectHandle {
public:
    ObjectHandle(entt::entity ent){
        m_Handle = ent;
    };

    Object GetAsObject(){
        return Object(m_Handle);
    };

    operator bool(){
        return Registry::Get().valid(m_Handle);
    };



private:
    entt::entity m_Handle = entt::null;

    
};