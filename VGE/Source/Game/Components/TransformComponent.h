#pragma once

#include "Common.h"
#include "ECS/Entity.h"
#include "ECS/Coordinator.h"

namespace vge
{
	struct TransformComponent
	{
	public:
		inline static TransformComponent* GetFrom(const Entity& entity) { return GCoordinator->GetComponent<TransformComponent>(entity); }

	public:
		glm::vec3 Translation = glm::vec3(0.0f);
		glm::vec3 Rotation = glm::vec3(0.0f);
		glm::vec3 Scale = glm::vec3(1.0f);

	public:
		// Matrix corresponds to Translate * Ry * Rx * Rz * Scale.
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3).
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix.
		glm::mat4 GetMat4() const;

		inline void ClampRotation(float degrees)
		{
			Rotation.x = glm::mod(Rotation.x, degrees);
			Rotation.y = glm::mod(Rotation.y, degrees);
			Rotation.z = glm::mod(Rotation.z, degrees);
		}
	};
}
