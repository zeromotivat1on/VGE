#pragma once

#include <string>
#include <vector>

#include "ECS/Entity.h"
#include "ECS/Component.h"
#include "ECS/Components/TransformComponent.h"

namespace vge
{
// Base component for hierarchy manipulation, has only transform component and tree-like needed data.
struct NodeComponent
{
    size_t Id;
    std::string Name;
    NodeComponent* Parent;
    TransformComponent Transform;
    std::vector<NodeComponent*> Children;
    std::vector<ComponentType> ComponentTypes;
};
}   // namespace vge
