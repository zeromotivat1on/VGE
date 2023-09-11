#pragma once

#include "Common.h"

namespace vge
{
	class Camera
	{
	public:
		Camera() = default;

		void SetViewDirection(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up);
		void SetViewTarget(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);
		void SetViewYXZ(const glm::vec3& position, const glm::vec3& rotation);

		void SetOrthographicProjection(float l, float r, float t, float b, float n, float f);
		void SetPerspectiveProjection(float fovy, float aspect, float n, float f);

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
