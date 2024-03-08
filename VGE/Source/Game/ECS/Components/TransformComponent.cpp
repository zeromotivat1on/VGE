#include "TransformComponent.h"

#include "NodeComponent.h"
#include "Types.h"
#include "ECS/Coordinator.h"

glm::mat4 vge::GetMat4(const TransformComponent& transform)
{
	return glm::translate(glm::mat4(1.0), transform.Translation) *
		   glm::mat4_cast(transform.Rotation) *
		   glm::scale(glm::mat4(1.0), transform.Scale);
	
	const f32 c1 = glm::cos(glm::radians(transform.Rotation.y));
	const f32 s1 = glm::sin(glm::radians(transform.Rotation.y));
	const f32 c2 = glm::cos(glm::radians(transform.Rotation.x));
	const f32 s2 = glm::sin(glm::radians(transform.Rotation.x));
	const f32 c3 = glm::cos(glm::radians(transform.Rotation.z));
	const f32 s3 = glm::sin(glm::radians(transform.Rotation.z));

	return glm::mat4
	{
		{
			transform.Scale.x * (c1 * c3 + s1 * s2 * s3),
			transform.Scale.x * (c2 * s3),
			transform.Scale.x * (c1 * s2 * s3 - c3 * s1),
			0.0f,     
		},
		{
			transform.Scale.y * (c3 * s1 * s2 - c1 * s3),
			transform.Scale.y * (c2 * c3),
			transform.Scale.y * (c1 * c3 * s2 + s1 * s3),
			0.0f,     
		},
		{
			transform.Scale.z * (c2 * s1),
			transform.Scale.z * (-s2),
			transform.Scale.z * (c1 * c2),
			0.0f,
		},
		{
			transform.Translation.x,
			transform.Translation.y,
			transform.Translation.z,
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

glm::mat4 vge::GetWorldMat4(TransformComponent& transform, bool update)
{
	if (!update)
	{
		return transform.WorldMatrix;
	}

	transform.WorldMatrix = GetMat4(transform);

	if (transform.Node)
	{
		if (NodeComponent* parent = transform.Node->Parent)
		{
			auto& parentTransform = ecs::GetComponent<Transform>(parent->Owner)
		}
	}
}
