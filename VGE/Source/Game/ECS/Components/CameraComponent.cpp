#include "CameraComponent.h"
#include "NodeComponent.h"
#include "TransformComponent.h"
#include "Core/Error.h"

glm::mat4 vge::CameraComponent::GetViewMat4()
{
    return glm::inverse(Node->Transform.GetWorldMat4());
}

glm::mat4 vge::CameraComponent::GetProjMat4()
{
    const auto& projData = ProjData;
    
    switch (ViewType)
    {
    case CameraViewType::Orthographic:
        return glm::ortho(
            projData[0],
            projData[1],
            projData[2],
            projData[3],
            projData[4],
            projData[5]);
        
    case CameraViewType::Perspective:
        return glm::perspective(
            projData[0],
            projData[1],
            projData[2],
            projData[3]);
        
    default:
        ENSURE_MSG(false, "Unknown camera view type used %d.", ViewType);
    }
}

vge::CameraComponent vge::CameraComponent::CreateDefault()
{
    CameraComponent camera = {};
    camera.ProjData[0] = 1.0f;
    camera.ProjData[1] = 1.77f;
    camera.ProjData[2] = 0.1f;
    camera.ProjData[3] = 1000.0f;
    camera.ProjData[4] = 0.0f;
    camera.ProjData[5] = 0.0f;

    return camera;
}
