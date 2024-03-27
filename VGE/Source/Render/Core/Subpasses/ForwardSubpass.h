#pragma once

#include "Core/Subpasses/GeometrySubpass.h"

// This value is per type of light that we feed into the shader.
#define MAX_FORWARD_LIGHT_COUNT 8

namespace vge
{
class Scene;
struct CameraComponent;

struct alignas(16) ForwardLights
{
	Light DirectionalLights[MAX_FORWARD_LIGHT_COUNT];
	Light PointLights[MAX_FORWARD_LIGHT_COUNT];
	Light SpotLights[MAX_FORWARD_LIGHT_COUNT];
};

/**
 * @brief This subpass is responsible for rendering a Scene
 */
class ForwardSubpass : public GeometrySubpass
{
public:
	ForwardSubpass(RenderContext&, ShaderSource&& vertex, ShaderSource&& fragment, Scene&, CameraComponent&);

	virtual ~ForwardSubpass() = default;

public:
	void Prepare() override;
	void Draw(CommandBuffer&) override;
};

} // namespace vge
