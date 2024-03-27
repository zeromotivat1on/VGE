#pragma once

#include "ECS/Component.h"
#include "ECS/Components/AABBComponent.h"

namespace vge
{
struct NodeComponent;
struct MeshComponent;
    
struct ModelComponent : public Component
{
    AABBComponent Bounds;
    std::vector<NodeComponent*> Nodes;
    std::vector<MeshComponent*> Meshes;
};
} // namespace vge
