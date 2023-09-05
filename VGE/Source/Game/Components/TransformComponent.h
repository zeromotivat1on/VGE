#pragma once

#include "Common.h"

namespace vge
{
	struct TransformComponent
	{
		glm::vec3 Location;
		glm::vec3 Rotation;
		glm::vec3 Scale;
		float RotationAngle; // degrees
	};

	inline glm::mat4 GetTransformMatrix(const TransformComponent& transformComponent)
	{
		glm::mat4 matrix(1.0f);
		matrix = glm::translate(matrix, transformComponent.Location);
		matrix = glm::rotate(matrix, glm::radians(transformComponent.RotationAngle), transformComponent.Rotation);
		matrix = glm::scale(matrix, transformComponent.Scale);

		return matrix;
	}
}
