#pragma once

#include "GlmCommon.h"
#include "Core/Subpass.h"
#include "Core/Error.h"

namespace vge
{
class Scene;
class Node;
struct NodeComponent;
struct MeshComponent;
struct ModelComponent;
struct CameraComponent;

// Global uniform structure for base shader.
struct alignas(16) GlobalUniform
{
	glm::mat4 Model;
	glm::mat4 CameraViewProj;
	glm::vec3 CameraPosition;
};

// PBR material uniform for base shader.
struct PBRMaterialUniform
{
	glm::vec4 BaseColorFactor;
	float MetallicFactor;
	float RoughnessFactor;
};

// This subpass is responsible for rendering a Scene.
class GeometrySubpass : public Subpass
{
public:
	GeometrySubpass(RenderContext&, ShaderSource&& vertex, ShaderSource&& fragment, Scene&, CameraComponent&);

	virtual ~GeometrySubpass() = default;

public:
	inline void SetThreadIndex(u32 index) { _ThreadIndex = index; }
	
	virtual void Prepare() override;
	virtual void Draw(CommandBuffer&) override;

protected:
	void DrawMesh(CommandBuffer&, MeshComponent&, VkFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE);
	virtual void UpdateUniform(CommandBuffer&, NodeComponent&, size_t threadIndex);
	virtual void DrawMeshCommand(CommandBuffer&, MeshComponent&);
	virtual void PreparePushConstants(CommandBuffer&, MeshComponent&);
	virtual void PreparePipelineState(CommandBuffer&, VkFrontFace, bool doubleSidedMaterial);
	virtual PipelineLayout& PreparePipelineLayout(CommandBuffer&, const std::vector<ShaderModule*>&);

	// Sorts objects based on distance from camera and classifies them into opaque and transparent in the arrays provided.
	void GetSortedNodes(std::multimap<float, std::pair<NodeComponent*, MeshComponent*>>& opaqueNodes,
						std::multimap<float, std::pair<NodeComponent*, MeshComponent*>>& transparentNodes);

protected:
	Scene& _Scene;
	CameraComponent& _Camera;
	u32 _ThreadIndex = 0;
	std::vector<ModelComponent*> _Models;
	RasterizationState _BaseRasterizationState = {};
};
}	// namespace vge
