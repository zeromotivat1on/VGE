#include "TransformComponent.h"
#include "NodeComponent.h"
#include <glm/gtx/matrix_decompose.hpp>

glm::mat4 vge::TransformComponent::GetMat4() const
{
	return glm::translate(glm::mat4(1.0), _Translation) *
		   glm::mat4_cast(_Rotation) *
		   glm::scale(glm::mat4(1.0), _Scale);
}

glm::mat4 vge::TransformComponent::GetWorldMat4()
{
	UpdateWorldTransform();
	return _WorldMatrix;
}

void vge::TransformComponent::SetMatrix(const glm::mat4& matrix)
{
	glm::vec3 skew;
	glm::vec4 perspective;
	CHECK(glm::decompose(matrix, _Scale, _Rotation, _Translation, skew, perspective));

	InvalidateWorldMatrix();
}

void vge::TransformComponent::UpdateWorldTransform()
{
	if (!_UpdateWorldMatrix)
	{
		return;
	}

	_WorldMatrix = GetMat4();

	if (_Node)
	{
		if (NodeComponent* parent = _Node->Parent)
		{
			auto& parentTransform = parent->GetComponent<TransformComponent>();
			_WorldMatrix = parentTransform.GetWorldMat4() * _WorldMatrix;
		}
	}

	_UpdateWorldMatrix = false;
}
