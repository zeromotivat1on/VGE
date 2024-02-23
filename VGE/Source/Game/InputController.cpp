#include "InputController.h"
#include "Render/Window.h"
#include "ECS/Coordinator.h"
#include "Components/TransformComponent.h"

void vge::InputController::MoveInPlaneXZ(f32 deltaTime, const Entity& entity)
{
	GLFWwindow* window = m_Window->GetHandle();
	auto& transform = ecs::GetComponent<TransformComponent>(entity);

	// Handle rotation.
	{
		glm::vec3 normRot = GetNormRotation();
		if (glm::dot(normRot, normRot) > FLT_EPSILON)
		{
			transform.Rotation += deltaTime * m_RotateSpeed * normRot;
			transform.ClampRotation(glm::degrees(glm::two_pi<f32>()));
			LOG_RAW("Rotation = {%.2f, %.2f, %.2f}\n", transform.Rotation.x, transform.Rotation.y, transform.Rotation.z);
		}
	}

	// Handle movement.
	{
		glm::vec3 normMove = GetNormTranslation(transform.Rotation);
		if (glm::dot(normMove, normMove) > FLT_EPSILON)
		{
			transform.Translation += deltaTime * m_MoveSpeed * normMove;
			LOG_RAW("Translation = {%.2f, %.2f, %.2f}\n", transform.Translation.x, transform.Translation.y, transform.Translation.z);
		}
	}
}

glm::vec3 vge::InputController::GetNormRotation()
{
	glm::vec3 rotate(0.0f);
	if (m_Window->IsKeyPressed(m_KeyboardKeys.LookRight))	rotate.y += 1.0f;
	if (m_Window->IsKeyPressed(m_KeyboardKeys.LookLeft))	rotate.y -= 1.0f;
	if (m_Window->IsKeyPressed(m_KeyboardKeys.LookUp))		rotate.x += 1.0f;
	if (m_Window->IsKeyPressed(m_KeyboardKeys.LookDown))	rotate.x -= 1.0f;

	if (m_InvertVerticalAxis)
	{
		rotate.x *= -1.0f;
	}

	return glm::normalize(rotate);
}

glm::vec3 vge::InputController::GetNormTranslation(const glm::vec3& rotation)
{
	const f32 yaw = rotation.y;
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

	return glm::normalize(moveDir);
}

