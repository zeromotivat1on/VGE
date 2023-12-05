#include "InputController.h"
#include "Renderer/Window.h"
#include "ECS/Coordinator.h"
#include "Components/TransformComponent.h"

namespace vge
{
	void UpdateRotation(vge::TransformComponent* outTransform, f32 deltaTime, f32 speed, const glm::vec3& rotationVector)
	{
		if (!outTransform)
		{
			return;
		}

		if (glm::dot(rotationVector, rotationVector) > FLT_EPSILON)
		{
			outTransform->Rotation += speed * deltaTime * glm::normalize(rotationVector);
			LOG_RAW("Rotation = {%.2f, %.2f, %.2f}\n", outTransform->Rotation.x, outTransform->Rotation.y, outTransform->Rotation.z);
		}
	}

	void UpdateTranslation(vge::TransformComponent* outTransform, f32 deltaTime, f32 speed, const glm::vec3& moveVector)
	{
		if (!outTransform)
		{
			return;
		}

		if (glm::dot(moveVector, moveVector) > FLT_EPSILON)
		{
			outTransform->Translation += speed * deltaTime * glm::normalize(moveVector);
			LOG_RAW("Translation = {%.2f, %.2f, %.2f}\n", outTransform->Translation.x, outTransform->Translation.y, outTransform->Translation.z);
		}
	}
}

void vge::InputController::MoveInPlaneXZ(f32 deltaTime, const Entity& entity)
{
	auto* transformComponent = GCoordinator->GetComponent<TransformComponent>(entity);

	if (!transformComponent)
	{
		return;
	}

	GLFWwindow* window = m_Window->GetHandle();

	// Handle rotation.
	{
		glm::vec3 rotate(0.0f);
		if (m_Window->IsKeyPressed(m_KeyboardKeys.LookRight))	rotate.y += 1.0f;
		if (m_Window->IsKeyPressed(m_KeyboardKeys.LookLeft))	rotate.y -= 1.0f;
		if (m_Window->IsKeyPressed(m_KeyboardKeys.LookUp))		rotate.x += m_InvertVerticalAxis ? -1.0f : 1.0f;
		if (m_Window->IsKeyPressed(m_KeyboardKeys.LookDown))	rotate.x -= m_InvertVerticalAxis ? -1.0f : 1.0f;

		UpdateRotation(transformComponent, deltaTime, m_RotateSpeed, rotate);
		transformComponent->ClampRotation(glm::degrees(glm::two_pi<f32>()));
	}

	// Handle movement.
	{
		const f32 yaw = transformComponent->Rotation.y;
		const f32 yawRad = glm::radians(yaw);
		const glm::vec3 forwardDir(glm::sin(yawRad), 0.0f, glm::cos(yawRad));
		const glm::vec3 rightDir(forwardDir.z, 0.0f, -forwardDir.x);
		const glm::vec3 upDir(0.0f, 1.0f, 0.0f);

		glm::vec3 moveDir(0.0f);
		if (m_Window->IsKeyPressed(m_KeyboardKeys.MoveForward))		moveDir += forwardDir;
		if (m_Window->IsKeyPressed(m_KeyboardKeys.MoveBackward))	moveDir -= forwardDir;
		if (m_Window->IsKeyPressed(m_KeyboardKeys.MoveRight))		moveDir += rightDir;
		if (m_Window->IsKeyPressed(m_KeyboardKeys.MoveLeft))		moveDir -= rightDir;
		if (m_Window->IsKeyPressed(m_KeyboardKeys.MoveUp))			moveDir += upDir;
		if (m_Window->IsKeyPressed(m_KeyboardKeys.MoveDown))		moveDir -= upDir;

		UpdateTranslation(transformComponent, deltaTime, m_MoveSpeed, moveDir);
	}
}

