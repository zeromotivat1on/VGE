#include "TransformComponent.h"

glm::mat4 vge::TransformComponent::GetMat4() const
{
	const float c1 = glm::cos(glm::radians(Rotation.y));
	const float s1 = glm::sin(glm::radians(Rotation.y));
	const float c2 = glm::cos(glm::radians(Rotation.x));
	const float s2 = glm::sin(glm::radians(Rotation.x));
	const float c3 = glm::cos(glm::radians(Rotation.z));
	const float s3 = glm::sin(glm::radians(Rotation.z));

	return glm::mat4
	{
		{
			Scale.x * (c1 * c3 + s1 * s2 * s3),
			Scale.x * (c2 * s3),
			Scale.x * (c1 * s2 * s3 - c3 * s1),
			0.0f,
		},
		{
			Scale.y * (c3 * s1 * s2 - c1 * s3),
			Scale.y * (c2 * c3),
			Scale.y * (c1 * c3 * s2 + s1 * s3),
			0.0f,
		},
		{
			Scale.z * (c2 * s1),
			Scale.z * (-s2),
			Scale.z * (c1 * c2),
			0.0f,
		},
		{
			Translation.x,
			Translation.y,
			Translation.z,
			1.0f
		}
	};


	// GLM version.
	//glm::mat4 matrix(1.0f);
	//matrix = glm::translate(matrix, Translation);
	//matrix = glm::rotate(matrix, glm::radians(Rotation.y), Rotation);
	//matrix = glm::rotate(matrix, glm::radians(Rotation.x), Rotation);
	//matrix = glm::rotate(matrix, glm::radians(Rotation.z), Rotation);
	//matrix = glm::scale(matrix, Scale);
	//return matrix;
}
