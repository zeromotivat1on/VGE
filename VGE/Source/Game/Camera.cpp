#include "Camera.h"

void vge::Camera::SetViewDirection(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up /*=glm::vec3(0.0f, 1.0f, 0.0f)*/)
{
	m_ViewMatrix = glm::mat4(1.0f);

#if USE_CUSTOM_MATRIX_CALCS
	const glm::vec3 w = { glm::normalize(direction) };
	const glm::vec3 u = { glm::normalize(glm::cross(w, up)) };
	const glm::vec3 v = { glm::cross(w, u) };

	m_ViewMatrix[0][0] = u.x;
	m_ViewMatrix[1][0] = u.y;
	m_ViewMatrix[2][0] = u.z;
	m_ViewMatrix[0][1] = v.x;
	m_ViewMatrix[1][1] = v.y;
	m_ViewMatrix[2][1] = v.z;
	m_ViewMatrix[0][2] = w.x;
	m_ViewMatrix[1][2] = w.y;
	m_ViewMatrix[2][2] = w.z;
	m_ViewMatrix[3][0] = -glm::dot(u, position);
	m_ViewMatrix[3][1] = -glm::dot(v, position);
	m_ViewMatrix[3][2] = -glm::dot(w, position);
#else
	m_ViewMatrix = glm::lookAt(position, glm::normalize(direction), up);
#endif
}

void vge::Camera::SetViewTarget(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up /*=glm::vec3(0.0f, 1.0f, 0.0f)*/)
{
	SetViewDirection(position, target - position, up);
}

void vge::Camera::SetViewYXZ(const glm::vec3& position, const glm::vec3& rotation)
{
	const float c1 = glm::cos(rotation.y);
	const float s1 = glm::sin(rotation.y);
	const float c2 = glm::cos(rotation.x);
	const float s2 = glm::sin(rotation.x);
	const float c3 = glm::cos(rotation.z);
	const float s3 = glm::sin(rotation.z);
	
	const glm::vec3 u = { (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
	const glm::vec3 v = { (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
	const glm::vec3 w = { (c2 * s1), (-s2), (c1 * c2) };

	m_ViewMatrix = glm::mat4(1.0f);
	m_ViewMatrix[0][0] = u.x;
	m_ViewMatrix[1][0] = u.y;
	m_ViewMatrix[2][0] = u.z;
	m_ViewMatrix[0][1] = v.x;
	m_ViewMatrix[1][1] = v.y;
	m_ViewMatrix[2][1] = v.z;
	m_ViewMatrix[0][2] = w.x;
	m_ViewMatrix[1][2] = w.y;
	m_ViewMatrix[2][2] = w.z;
	m_ViewMatrix[3][0] = -glm::dot(u, position);
	m_ViewMatrix[3][1] = -glm::dot(v, position);
	m_ViewMatrix[3][2] = -glm::dot(w, position);
}

void vge::Camera::SetOrthographicProjection(float l, float r, float t, float b, float n, float f) 
{
	m_ProjectionMatrix = glm::mat4(1.0f);

#if USE_CUSTOM_MATRIX_CALCS
	m_ProjectionMatrix[0][0] = 2.f / (r - l);
	m_ProjectionMatrix[1][1] = 2.f / (b - t);
	m_ProjectionMatrix[2][2] = 1.f / (f - n);
	m_ProjectionMatrix[3][0] = -(r + l) / (r - l);
	m_ProjectionMatrix[3][1] = -(b + t) / (b - t);
	m_ProjectionMatrix[3][2] = -n / (f - n);
#else
	m_ProjectionMatrix = glm::ortho(l, r, b, t, n, f);
#endif

	if (m_InvertProjectionYAxis)
	{
		m_ProjectionMatrix[1][1] *= -1;
	}
}

void vge::Camera::SetPerspectiveProjection(float fovy, float aspect, float n, float f) 
{
	ASSERT(glm::abs(aspect - FLT_EPSILON) > 0.0f);

	m_ProjectionMatrix = glm::mat4(0.0f);

#if USE_CUSTOM_MATRIX_CALCS
	const float tanHalfFovy = std::tan(fovy / 2.f);
	m_ProjectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
	m_ProjectionMatrix[1][1] = 1.f / (tanHalfFovy);
	m_ProjectionMatrix[2][2] = f / (f - n);
	m_ProjectionMatrix[2][3] = 1.f;
	m_ProjectionMatrix[3][2] = -(f * n) / (f - n);
#else
	m_ProjectionMatrix = glm::perspective(glm::radians(fovy), aspect, n, f);
#endif

	if (m_InvertProjectionYAxis)
	{
		m_ProjectionMatrix[1][1] *= -1; 
	}
}