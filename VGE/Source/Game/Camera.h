#pragma once

#include "Common.h"

namespace vge
{
	class Camera
	{
	public:
		Camera() = default;

		void SetOrthographicProjection(float l, float r, float t, float b, float n, float f);
		void SetPerspectiveProjection(float fovy, float aspect, float n, float f);

		inline const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

	private:
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
	};
}
