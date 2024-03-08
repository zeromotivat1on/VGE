#include "CameraComponent.h"
#include "NodeComponent.h"
#include "TransformComponent.h"
#include "Core/Error.h"

glm::mat4 vge::GetViewMat4(const CameraComponent& camera)
{
    auto& transform = camera.Node->Transform;
    return glm::inverse(GetWorldMat4(transform));
}

glm::mat4 vge::GetProjMat4(const CameraComponent& camera)
{
    const auto& projData = camera.ProjData;
    
    switch (camera.ViewType)
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
        ENSURE_MSG(false, "Unknown camera type %d", camera.ViewType);
    }
}
