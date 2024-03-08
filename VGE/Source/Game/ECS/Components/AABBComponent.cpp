#include "AABBComponent.h"

void vge::Update(AABBComponent& aabb, const glm::vec3& point)
{
    aabb.Min = glm::min(aabb.Min, point);
    aabb.Max = glm::max(aabb.Max, point);
}

void vge::Update(AABBComponent& aabb, const std::vector<glm::vec3>& vertices, const std::vector<u32>& indices)
{
    // Check if submesh is indexed.
    if (!indices.empty())
    {
        // Update bounding box for each indexed vertex.
        for (const u32 index : indices)
        {
            Update(aabb, vertices[index]);
        }
    }
    else
    {
        // Update bounding box for each vertex.
        for (const glm::vec3& vertex : vertices)
        {
            Update(aabb, vertex);
        }
    }
}

void vge::Transform(AABBComponent& aabb, const glm::mat4& transform)
{
    aabb.Min = aabb.Max = glm::vec4(aabb.Min, 1.0f) * transform;

    // Update bounding box for the remaining 7 corners of the box.
    Update(aabb, glm::vec4(aabb.Min.x, aabb.Min.y, aabb.Max.z, 1.0f) * transform);
    Update(aabb, glm::vec4(aabb.Min.x, aabb.Max.y, aabb.Min.z, 1.0f) * transform);
    Update(aabb, glm::vec4(aabb.Min.x, aabb.Max.y, aabb.Max.z, 1.0f) * transform);
    Update(aabb, glm::vec4(aabb.Max.x, aabb.Min.y, aabb.Min.z, 1.0f) * transform);
    Update(aabb, glm::vec4(aabb.Max.x, aabb.Min.y, aabb.Max.z, 1.0f) * transform);
    Update(aabb, glm::vec4(aabb.Max.x, aabb.Max.y, aabb.Min.z, 1.0f) * transform);
    Update(aabb, glm::vec4(aabb.Max, 1.0f) * transform);
}

void vge::Reset(AABBComponent& aabb)
{
    aabb.Min = std::numeric_limits<glm::vec3>::max();
    aabb.Max = std::numeric_limits<glm::vec3>::min();
}

glm::vec3 vge::GetScale(const AABBComponent& aabb)
{
    return (aabb.Max - aabb.Min);
}

glm::vec3 vge::GetCenter(const AABBComponent& aabb)
{
    return (aabb.Min + aabb.Max) * 0.5f;
}
