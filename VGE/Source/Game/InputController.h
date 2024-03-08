#pragma once

#include "Common.h"
#include "ECS/Entity.h"
#include "Render/RenderCommon.h"
#include "Core/Error.h"

namespace vge
{
	class Window;

	struct KeyboardMappings
	{
		i32 MoveLeft = GLFW_KEY_A;
		i32 MoveRight = GLFW_KEY_D;
		i32 MoveForward = GLFW_KEY_W;
		i32 MoveBackward = GLFW_KEY_S;
		i32 MoveUp = GLFW_KEY_E;
		i32 MoveDown = GLFW_KEY_Q;
		i32 LookLeft = GLFW_KEY_LEFT;
		i32 LookRight = GLFW_KEY_RIGHT;
		i32 LookUp = GLFW_KEY_UP;
		i32 LookDown = GLFW_KEY_DOWN;
	};

	class InputController
	{
	public:
		InputController() = default;
		InputController(Window* window) : m_Window(window) { ENSURE(m_Window); }

		void MoveInPlaneXZ(f32 deltaTime, const Entity& entity);

		inline void SetMoveSpeed(f32 moveSpeed) { m_MoveSpeed = moveSpeed; }
		inline void SetRotateSpeed(f32 rotateSpeed) { m_RotateSpeed = rotateSpeed ;}
		inline void ShouldInvertVerticalAxis(bool invert) { m_InvertVerticalAxis = invert; }

	private:
		const Window* m_Window = nullptr;
		f32 m_RotateSpeed = 0.0f;
		f32 m_MoveSpeed = 0.0f;
		bool m_InvertVerticalAxis = false;
		KeyboardMappings m_KeyboardKeys = {};

	private:
		glm::vec3 GetNormRotation();
		glm::vec3 GetNormTranslation(const glm::vec3& rotation);
	};
}
