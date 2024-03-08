#pragma once

#include <vector>
#include "GlmCommon.h"
#include "Types.h"

namespace vge
{
struct AABBComponent
{
    glm::vec3 Min;
    glm::vec3 Max;
};

void Update(AABBComponent& aabb, const glm::vec3& point);
void Update(AABBComponent& aabb, const std::vector<glm::vec3>& vertices, const std::vector<u32>& indices);
void Transform(AABBComponent& aabb, const glm::mat4& transform);
void Reset(AABBComponent& aabb);
glm::vec3 GetScale(const AABBComponent& aabb);
glm::vec3 GetCenter(const AABBComponent& aabb);
} // namespace vge
