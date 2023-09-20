#pragma once

#include "Common.h"

namespace vge
{
	struct TransformComponent
	{
		glm::vec3 Translation = glm::vec3(0.0f);
		glm::vec3 Rotation = glm::vec3(0.0f);
		glm::vec3 Scale = glm::vec3(1.0f);
	};

	// Note: custom mode has visually different rotation in case of several rotation axis at a time.
	glm::mat4 GetMat4(const TransformComponent& transformComponent);
}
