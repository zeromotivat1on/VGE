#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "ECS/Component.h"
#include "ECS/Coordinator.h"
#include "ECS/Components/NodeComponent.h"

#define ENSURE_NOT_NODE(type) ENSURE(type != ecs::GetComponentType<NodeComponent>());

namespace vge
{
struct NodeComponent;

// Holds refs to components on scene. Actual components' data stored in ECS Coordinator.
class Scene
{
public:
    Scene() = default;
    Scene(const std::string& name);

public:
    inline void SetName(const std::string& name) { _Name = name; }
    inline void SetRoot(NodeComponent& root) { _Root = &root; }
    inline const std::string& GetName() const { return _Name; }
    inline NodeComponent& GetRoot() { return *_Root; }
    
    inline void AddNode(NodeComponent& node) { _Nodes.push_back(&node); }
    inline void AddChild(NodeComponent& node) { _Root->Children.push_back(&node); }

    inline void AddComponent(ComponentType type, Component& component)
    {
        ENSURE_NOT_NODE(type);
        _Components[type].push_back(&component);
    }
    
    inline void AddComponent(ComponentType type, Component& component, NodeComponent& node)
    {
        ENSURE_NOT_NODE(type);
        node.Components.at(type) = &component;
        AddComponent(type, component);
    }
    
    inline const std::vector<Component*>& GetComponents(ComponentType type) const { return _Components.at(type); }

    inline bool HasComponent(ComponentType type) const
    {
        const auto it = _Components.find(type);
        return (it != _Components.end() && !it->second.empty()); 
    }

    template<typename T>
    bool HasComponent() const { return HasComponent(ecs::GetComponentType<T>()); }

    template<typename T>
    void AddComponent(Component& component) { AddComponent(ecs::GetComponentType<T>(), component); }
    
    template<typename T>
    std::vector<T*> GetComponents() const
    {
        std::vector<T*> result;
        const ComponentType componentType = ecs::GetComponentType<T>();
        
        if (HasComponent(componentType))
        {
            const auto& components = GetComponents(componentType);
            
            result.resize(components.size());
            std::transform(components.begin(), components.end(), result.begin(),
                           [](Component* component) -> T*
                           {
                               return static_cast<T*>(component);
                           });
        }

        return result;
    }

    NodeComponent* FindNode(const std::string& nodeName);
    
private:
    std::string _Name;
    NodeComponent* _Root = nullptr;
    std::vector<NodeComponent*> _Nodes;
    std::unordered_map<ComponentType, std::vector<Component*>> _Components;
};
}   // namespace vge
