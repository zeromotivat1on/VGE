#pragma once

#include <string>
#include <vector>

#include "ECS/Component.h"
#include "ECS/Coordinator.h"
#include "ECS/Components/TransformComponent.h"

namespace vge
{
// Base component for hierarchy manipulation, has only transform component and tree-like needed data.

struct NodeComponent : public Component
{
public:
    size_t Id;
    std::string Name;
    NodeComponent* Parent;
    TransformComponent Transform;
    std::vector<NodeComponent*> Children;
    std::unordered_map<ComponentType, Component*> Components;

public:
    template <typename T>
    T& GetComponent()
    {
        return *static_cast<T*>(Components.at(ecs::GetComponentType<T>()));
    }
};
}   // namespace vge
