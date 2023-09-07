#include "Camera.h"

void vge::Camera::SetOrthographicProjection(float l, float r, float t, float b, float n, float f) 
{
#if REPLACE_GLM_WITH_CUSTOM_CALCS
	m_ProjectionMatrix = glm::mat4{ 1.0f };
	m_ProjectionMatrix[0][0] = 2.f / (r - l);
	m_ProjectionMatrix[1][1] = 2.f / (b - t);
	m_ProjectionMatrix[2][2] = 1.f / (f - n);
	m_ProjectionMatrix[3][0] = -(r + l) / (r - l);
	m_ProjectionMatrix[3][1] = -(b + t) / (b - t);
	m_ProjectionMatrix[3][2] = -n / (f - n);
#else
	m_ProjectionMatrix = glm::ortho(l, r, b, t, n, f);
#endif
}

void vge::Camera::SetPerspectiveProjection(float fovy, float aspect, float n, float f) 
{
	ASSERT(glm::abs(aspect - FLT_EPSILON) > 0.0f);

#if REPLACE_GLM_WITH_CUSTOM_CALCS
	const float tanHalfFovy = std::tan(fovy / 2.f);
	m_ProjectionMatrix = glm::mat4{ 0.0f };
	m_ProjectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
	m_ProjectionMatrix[1][1] = 1.f / (tanHalfFovy);
	m_ProjectionMatrix[2][2] = f / (f - n);
	m_ProjectionMatrix[2][3] = 1.f;
	m_ProjectionMatrix[3][2] = -(f * n) / (f - n);

#else
	m_ProjectionMatrix = glm::perspective(glm::radians(fovy), aspect, n, f);
#endif
}