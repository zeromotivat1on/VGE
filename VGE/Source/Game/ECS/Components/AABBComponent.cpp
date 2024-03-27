#include "AABBComponent.h"

void vge::AABBComponent::Update(const glm::vec3& point)
{
    Min = glm::min(Min, point);
    Max = glm::max(Max, point);
}

void vge::AABBComponent::Update(const std::vector<glm::vec3>& vertices, const std::vector<u32>& indices)
{
    // Check if submesh is indexed.
    if (!indices.empty())
    {
        // Update bounding box for each indexed vertex.
        for (const u32 index : indices)
        {
            Update(vertices[index]);
        }
    }
    else
    {
        // Update bounding box for each vertex.
        for (const glm::vec3& vertex : vertices)
        {
            Update(vertex);
        }
    }
}

void vge::AABBComponent::Transform(const glm::mat4& transform)
{
    Min = Max = glm::vec4(Min, 1.0f) * transform;

    // Update bounding box for the remaining 7 corners of the box.
    Update(glm::vec4(Min.x, Min.y, Max.z, 1.0f) * transform);
    Update(glm::vec4(Min.x, Max.y, Min.z, 1.0f) * transform);
    Update(glm::vec4(Min.x, Max.y, Max.z, 1.0f) * transform);
    Update(glm::vec4(Max.x, Min.y, Min.z, 1.0f) * transform);
    Update(glm::vec4(Max.x, Min.y, Max.z, 1.0f) * transform);
    Update(glm::vec4(Max.x, Max.y, Min.z, 1.0f) * transform);
    Update(glm::vec4(Max, 1.0f) * transform);
}

void vge::AABBComponent::Reset()
{
    Min = std::numeric_limits<glm::vec3>::max();
    Max = std::numeric_limits<glm::vec3>::min();
}
