#pragma once

#include "ECS/Components/AABBComponent.h"

namespace vge
{
struct NodeComponent;
struct MeshComponent;
    
struct ModelComponent
{
    AABBComponent Bounds;
    std::vector<NodeComponent*> Nodes;
    std::vector<MeshComponent*> Meshes;
};
} // namespace vge
