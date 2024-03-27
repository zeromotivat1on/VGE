#pragma once

#include "GlmCommon.h"
#include "ECS/Component.h"

namespace vge
{
struct NodeComponent;

enum class CameraViewType
{
    Perspective,
    Orthographic,
};
    
struct CameraComponent : public Component
{
public:
    static CameraComponent CreateDefault();

public:
    NodeComponent* Node = nullptr;
    glm::mat4 PreRotation = glm::mat4(1.0f);

    // Type of camera view. Ensure ProjData is correct for current view type.
    CameraViewType ViewType = CameraViewType::Perspective;

    // Data to use for projection matrix calculation.
    // Perspective order - [fov, aspect, near, far, x, x] (last 2 not used).
    // Orthographic order - [left, right, bottom, top, near, far]. 
    float ProjData[6];

public:
    glm::mat4 GetViewMat4();
    glm::mat4 GetProjMat4(); // ensure ProjData is correct
};
}	// namespace vge
