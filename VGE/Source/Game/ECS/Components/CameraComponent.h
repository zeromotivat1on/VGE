#pragma once

#include "GlmCommon.h"

namespace vge
{
struct NodeComponent;

enum class CameraViewType
{
    Perspective,
    Orthographic,
};
    
struct CameraComponent
{
    NodeComponent* Node = nullptr;
    glm::mat4 PreRotation = glm::mat4(1.0f);
    CameraViewType ViewType = CameraViewType::Perspective;

    // Data to use for projection matrix calculation.
    // Perspective order - [fov, aspect, far, near, x, x] (last 2 not used).
    // Orthographic order - [left, right, bottom, top, far, near]. 
    float ProjData[6];
};

glm::mat4 GetViewMat4(const CameraComponent&); // ensure ProjData is correct
glm::mat4 GetProjMat4(const CameraComponent&); // ensure ProjData is correct
}	// namespace vge
