#include "Camera.h"

void vge::Camera::SetViewDirection(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up /*=glm::vec3(0.0f, -1.0f, 0.0f)*/)
{
	const glm::vec3 w = { glm::normalize(direction) };
	const glm::vec3 u = { glm::normalize(glm::cross(w, up)) };
	const glm::vec3 v = { glm::cross(w, u) };

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

	// GLM version.
	//m_ViewMatrix = glm::lookAt(position, glm::normalize(direction), up);
}

void vge::Camera::SetViewTarget(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up /*=glm::vec3(0.0f, -1.0f, 0.0f)*/)
{
	SetViewDirection(position, target - position, up);
}

void vge::Camera::SetViewYXZ(const glm::vec3& position, const glm::vec3& rotation)
{
	const f32 c1 = glm::cos(glm::radians(rotation.y));
	const f32 s1 = glm::sin(glm::radians(rotation.y));
	const f32 c2 = glm::cos(glm::radians(rotation.x));
	const f32 s2 = glm::sin(glm::radians(rotation.x));
	const f32 c3 = glm::cos(glm::radians(rotation.z));
	const f32 s3 = glm::sin(glm::radians(rotation.z));
	
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

void vge::Camera::SetOrthographicProjection(f32 l, f32 r, f32 t, f32 b, f32 n, f32 f) 
{
	m_ProjectionMatrix = glm::mat4(1.0f);
	m_ProjectionMatrix[0][0] = 2.f / (r - l);
	m_ProjectionMatrix[1][1] = 2.f / (b - t);
	m_ProjectionMatrix[2][2] = 1.f / (f - n);
	m_ProjectionMatrix[3][0] = -(r + l) / (r - l);
	m_ProjectionMatrix[3][1] = -(b + t) / (b - t);
	m_ProjectionMatrix[3][2] = -n / (f - n);
	
	// GLM version.
	//m_ProjectionMatrix = glm::ortho(l, r, b, t, n, f);
	
	if (m_InvertProjectionYAxis)
	{
		m_ProjectionMatrix[1][1] *= -1;
	}
}

void vge::Camera::SetPerspectiveProjection(f32 fovy, f32 aspect, f32 n, f32 f) 
{
	ASSERT(glm::abs(aspect - FLT_EPSILON) > 0.0f);

	const f32 tanHalfFovy = std::tan(fovy / 2.f);

	m_ProjectionMatrix = glm::mat4(0.0f);
	m_ProjectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
	m_ProjectionMatrix[1][1] = 1.f / (tanHalfFovy);
	m_ProjectionMatrix[2][2] = f / (f - n);
	m_ProjectionMatrix[2][3] = 1.f;
	m_ProjectionMatrix[3][2] = -(f * n) / (f - n);
	
	// GLM version.
	//m_ProjectionMatrix = glm::perspective(glm::radians(fovy), aspect, n, f);
	
	if (m_InvertProjectionYAxis)
	{
		m_ProjectionMatrix[1][1] *= -1; 
	}
}