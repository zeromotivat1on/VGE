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

	// Matrix corresponds to Translate * Ry * Rx * Rz * Scale.
	// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3).
	// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix.
	// Note: custom mode has visually different rotation in case of several rotation axis at a time.
	glm::mat4 GetMat4(const TransformComponent& transformComponent);
}
