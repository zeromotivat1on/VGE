#pragma once

#include "GlmCommon.h"
#include "Core/Subpass.h"

// This value is per type of light that we feed into the shader
#define MAX_DEFERRED_LIGHT_COUNT 32

namespace vge
{
class Scene;
struct Light;
struct CameraComponent;

// Light uniform structure for lighting shader.
// Inverse view projection matrix and inverse resolution vector are used in lighting pass to reconstruct position from depth and frag coord.
struct alignas(16) LightUniform
{
	glm::mat4 InvViewProj;
	glm::vec2 InvResolution;
};

struct alignas(16) DeferredLights
{
	Light DirectionalLights[MAX_DEFERRED_LIGHT_COUNT];
	Light PointLights[MAX_DEFERRED_LIGHT_COUNT];
	Light SpotLights[MAX_DEFERRED_LIGHT_COUNT];
};

// Lighting pass of Deferred Rendering.
class LightingSubpass : public Subpass
{
public:
	LightingSubpass(RenderContext&, ShaderSource&& vertex, ShaderSource&& fragment, CameraComponent&, Scene&);

public:
	void Prepare() override;
	void Draw(CommandBuffer&) override;

private:
	CameraComponent& _Camera;
	Scene& _Scene;
	ShaderVariant _LightingVariant;
};
}	// namespace vge
