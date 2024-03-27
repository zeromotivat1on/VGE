#pragma once

#include "GlmCommon.h"
#include "ECS/Component.h"

namespace vge
{
struct NodeComponent;

struct TransformComponent : public Component
{
private:
	glm::vec3 _Translation = glm::vec3(0.0f);
	glm::quat _Rotation = glm::quat(1.0, 0.0, 0.0, 0.0);
	glm::vec3 _Scale = glm::vec3(1.0f);
	glm::mat4 _WorldMatrix = glm::mat4(1.0); // better call GetWorldMat4 instead of taking directly 
	NodeComponent* _Node = nullptr;
	bool _UpdateWorldMatrix = false;
	
public:
	inline const glm::vec3& GetTranslation() const { return _Translation; } 
	inline const glm::quat& GetRotation() const { return _Rotation; } 
	inline const glm::vec3& GetScale() const { return _Scale; } 
	inline NodeComponent* GetNode() const { return _Node; } 

	// Marks the world transform invalid if any of the local transform are changed or the parent world transform has changed.
	inline void InvalidateWorldMatrix() { _UpdateWorldMatrix = true; }

	inline void SetTranslation(const glm::vec3& t) { _Translation = t; InvalidateWorldMatrix(); }
	inline void SetRotation(const glm::quat& r) { _Rotation = r; InvalidateWorldMatrix(); }
	inline void SetScale(const glm::vec3& s) { _Scale = s; InvalidateWorldMatrix(); }
	inline void SetNode(NodeComponent* node) { _Node = node; }
	
	glm::mat4 GetMat4() const;
	glm::mat4 GetWorldMat4();

	void SetMatrix(const glm::mat4&);

private:
	// Update world matrix if invalidated.
	void UpdateWorldTransform();
};
}
