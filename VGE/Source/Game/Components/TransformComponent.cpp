#include "TransformComponent.h"

glm::mat4 vge::GetMat4(const TransformComponent& transformComponent)
{
#if USE_CUSTOM_MATRIX_CALCS
	// Matrix corresponds to Translate * Ry * Rx * Rz * Scale.
	// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3).
	// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix.

	const float c1 = glm::cos(glm::radians(transformComponent.Rotation.y));
	const float s1 = glm::sin(glm::radians(transformComponent.Rotation.y));
	const float c2 = glm::cos(glm::radians(transformComponent.Rotation.x));
	const float s2 = glm::sin(glm::radians(transformComponent.Rotation.x));
	const float c3 = glm::cos(glm::radians(transformComponent.Rotation.z));
	const float s3 = glm::sin(glm::radians(transformComponent.Rotation.z));

	return glm::mat4{
		{
			transformComponent.Scale.x * (c1 * c3 + s1 * s2 * s3),
			transformComponent.Scale.x * (c2 * s3),
			transformComponent.Scale.x * (c1 * s2 * s3 - c3 * s1),
			0.0f,
		},
		{
			transformComponent.Scale.y * (c3 * s1 * s2 - c1 * s3),
			transformComponent.Scale.y * (c2 * c3),
			transformComponent.Scale.y * (c1 * c3 * s2 + s1 * s3),
			0.0f,
		},
		{
			transformComponent.Scale.z * (c2 * s1),
			transformComponent.Scale.z * (-s2),
			transformComponent.Scale.z * (c1 * c2),
			0.0f,
		},
		{
			transformComponent.Translation.x,
			transformComponent.Translation.y,
			transformComponent.Translation.z,
			1.0f
		}
	};
#else
	glm::mat4 matrix(1.0f);
	matrix = glm::translate(matrix, transformComponent.Translation);
	matrix = glm::rotate(matrix, glm::radians(transformComponent.Rotation.y), transformComponent.Rotation);
	matrix = glm::rotate(matrix, glm::radians(transformComponent.Rotation.x), transformComponent.Rotation);
	matrix = glm::rotate(matrix, glm::radians(transformComponent.Rotation.z), transformComponent.Rotation);
	matrix = glm::scale(matrix, transformComponent.Scale);

	return matrix;
#endif
}
