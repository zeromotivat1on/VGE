#pragma once

#include "Common.h"

namespace vge
{
	// Current camera that observes the world.
	inline class Camera* GCamera = nullptr;
	
	class Camera
	{
	public:
		Camera() = default;

		void SetViewDirection(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up = glm::vec3(0.0f, -1.0f, 0.0f));
		void SetViewTarget(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up = glm::vec3(0.0f, -1.0f, 0.0f));
		void SetViewYXZ(const glm::vec3& position, const glm::vec3& rotation);

		void SetOrthographicProjection(f32 l, f32 r, f32 t, f32 b, f32 n, f32 f);
		void SetPerspectiveProjection(f32 fovy, f32 aspect, f32 n, f32 f);

		inline void ShouldInvertProjectionYAxis(bool invert) { m_InvertProjectionYAxis = invert; }
		inline const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		inline const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

	private:
		// Glm uses positive y-axis for up, but Vulkan uses it for down.
		// In order to obtain correct display, we should invert y-axis for Vulkan.
		bool m_InvertProjectionYAxis = false;
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
	};
}
