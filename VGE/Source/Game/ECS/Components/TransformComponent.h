#pragma once

#include "GlmCommon.h"

namespace vge
{
struct NodeComponent;
	
struct TransformComponent
{
	glm::vec3 Translation = glm::vec3(0.0f);
	glm::quat Rotation = glm::quat(1.0, 0.0, 0.0, 0.0);
	glm::vec3 Scale = glm::vec3(1.0f);
	glm::mat4 WorldMatrix = glm::mat4(1.0); // its better to call GetWorldMat4 instead of taking directly 
	NodeComponent* Node;
};

glm::mat4 GetMat4(const TransformComponent&);
glm::mat4 GetWorldMat4(TransformComponent&, bool update = true);
}
