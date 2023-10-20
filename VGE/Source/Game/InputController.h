#pragma once

#include "Common.h"
#include "ECS/Entity.h"
#include "Renderer/RenderCommon.h"

namespace vge
{
	class Window;

	struct KeyboardMappings
	{
		int32 MoveLeft = GLFW_KEY_A;
		int32 MoveRight = GLFW_KEY_D;
		int32 MoveForward = GLFW_KEY_W;
		int32 MoveBackward = GLFW_KEY_S;
		int32 MoveUp = GLFW_KEY_E;
		int32 MoveDown = GLFW_KEY_Q;
		int32 LookLeft = GLFW_KEY_LEFT;
		int32 LookRight = GLFW_KEY_RIGHT;
		int32 LookUp = GLFW_KEY_UP;
		int32 LookDown = GLFW_KEY_DOWN;
	};

	class InputController
	{
	public:
		InputController() = default;
		InputController(Window* window) : m_Window(window) { ENSURE(m_Window); }

		void MoveInPlaneXZ(float deltaTime, const Entity& entity);

		inline void SetMoveSpeed(float moveSpeed) { m_MoveSpeed = moveSpeed; }
		inline void SetRotateSpeed(float rotateSpeed) { m_RotateSpeed = rotateSpeed ;}
		inline void ShouldInvertVerticalAxis(bool invert) { m_InvertVerticalAxis = invert; }

	private:
		const Window* m_Window = nullptr;
		float m_RotateSpeed = 0.0f;
		float m_MoveSpeed = 0.0f;
		bool m_InvertVerticalAxis = false;
		KeyboardMappings m_KeyboardKeys = {};
	};
}
