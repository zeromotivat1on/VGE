#pragma once

#include <vector>
#include "GlmCommon.h"
#include "Types.h"
#include "ECS/Component.h"

namespace vge
{
struct AABBComponent : public Component
{
public:
    glm::vec3 Min;
    glm::vec3 Max;

public:
    inline glm::vec3 GetScale() const { return (Max - Min); }
    inline glm::vec3 GetCenter() const { return (Min + Max) * 0.5f; }
    
    void Update(const glm::vec3& point);
    void Update(const std::vector<glm::vec3>& vertices, const std::vector<u32>& indices);
    void Transform(const glm::mat4& transform);
    void Reset();
};
} // namespace vge
