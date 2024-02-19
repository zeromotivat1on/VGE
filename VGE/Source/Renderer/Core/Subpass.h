#pragma once

#include "Core/Common.h"
#include "Core/BufferPool.h"
#include "Core/ShaderModule.h"
#include "Core/RenderTarget.h"
#include "Core/PipelineState.h"
#include "Core/RenderContext.h"

namespace vge
{
class CommandBuffer;

struct alignas(16) Light
{
	glm::vec4 Position;         // position.w represents type of light
	glm::vec4 Color;            // color.w represents light intensity
	glm::vec4 Direction;        // direction.w represents range
	glm::vec2 Info;             // (only used for spot lights) info.x represents light inner cone angle, info.y represents light outer cone angle
};

struct LightingState
{
	std::vector<Light> DirectionalLights;
	std::vector<Light> PointLights;
	std::vector<Light> SpotLights;
	BufferAllocation LightBuffer;
};

/**
	* @brief Calculates the vulkan style projection matrix
	* @param proj The projection matrix
	* @return The vulkan style projection matrix
	*/
glm::mat4 VulkanStyleProjection(const glm::mat4& proj);

extern const std::vector<std::string> GLightTypeDefinitions;

// This class defines an interface for subpasses where they need to implement the draw function. It is used to construct a RenderPipeline
class Subpass
{
public:
	Subpass(RenderContext& renderContext, ShaderSource&& vertexShader, ShaderSource&& fragmentShader);

	COPY_CTOR_DEL(Subpass);
	Subpass(Subpass&&) = default;

	virtual ~Subpass() = default;

	COPY_OP_DEL(Subpass);
	MOVE_OP_DEL(Subpass);

public:
	virtual void Prepare() = 0; // prepares the shaders and shader variants for a subpass
	virtual void Draw(CommandBuffer& commandBuffer) = 0;

	// Updates the render target attachments with the ones stored in this subpass.
	// This function is called by the RenderPipeline before beginning the render pass and before proceeding with a new subpass.
	void UpdateRenderTargetAttachments(RenderTarget& render_target);

	inline RenderContext& GetRenderContext() { return _RenderContext; }
	inline LightingState& GetLightingState() { return _LightingState; }
	inline DepthStencilState& GetDepthStencilState() { return _DepthStencilState; }
	inline const ShaderSource& GetVertexShader() const { return _VertexShader; }
	inline const ShaderSource& GetFragmentShader() const { return _FragmentShader; }
	inline const std::vector<u32>& GetInputAttachments() const { return _InputAttachments; }
	inline const std::vector<u32>& GetOutputAttachments() const { return _OutputAttachments; }
	inline const std::vector<u32>& GetColorResolveAttachments() const { return _ColorResolveAttachments; }
	inline const bool GetDisableDepthStencilAttachment() const { return _DisableDepthStencilAttachment; }
	inline const u32 GetDepthStencilResolveAttachment() const { return _DepthStencilResolveAttachment; }
	inline const VkResolveModeFlagBits GetDepthStencilResolveMode() const { return _DepthStencilResolveMode; }
	inline const std::string& GetDebugName() const { return _DebugName; }

	inline void SetInputAttachments(std::vector<u32> input) { _InputAttachments = input; }
	inline void SetOutputAttachments(std::vector<u32> output) { _OutputAttachments = output; }
	inline void SetSampleCount(VkSampleCountFlagBits sample_count) { sample_count = sample_count; }
	inline void SetColorResolveAttachments(std::vector<u32> color_resolve) { _ColorResolveAttachments = color_resolve; }
	inline void SetDisableDepthStencilAttachment(bool disable_depth_stencil) { disable_depth_stencil = disable_depth_stencil; }
	inline void SetDepthStencilResolveAttachment(u32 depth_stencil_resolve) { _DepthStencilResolveAttachment = depth_stencil_resolve; }
	inline void SetDepthStencilResolveMode(VkResolveModeFlagBits mode) { _DepthStencilResolveMode = mode; }
	inline void SetDebugName(const std::string& name) { _DebugName = name; }

	/**
		* @brief Prepares the lighting state to have its lights
		*
		* @tparam A light structure that has 'DirectionalLights', 'PointLights' and 'spot_light' array fields defined.
		* @param scene_lights All of the light components from the scene graph
		* @param light_count The maximum amount of lights allowed for any given type of light.
		*/
	template <typename T>
	void AllocateLights(const std::vector<sg::Light*>& sceneLights, size_t lightCount)
	{
		ASSERT_MSG(sceneLights.size() <= (lightCount * sg::LightType::Max), "Exceeding max light capacity.");

		_LightingState.DirectionalLights.clear();
		_LightingState.PointLights.clear();
		_LightingState.SpotLights.clear();

		for (auto& sceneLight : sceneLights)
		{
			const auto& properties = sceneLight->get_properties();
			auto& transform = sceneLight->get_node()->get_transform();

			Light light{{transform.get_translation(), static_cast<float>(sceneLight->get_light_type())},
						{properties.color, properties.intensity},
						{transform.get_rotation() * properties.direction, properties.range},
						{properties.inner_cone_angle, properties.outer_cone_angle}};

			switch (sceneLight->get_light_type())
			{
			case sg::LightType::Directional:
			{
				if (_LightingState.DirectionalLights.size() < lightCount)
				{
					_LightingState.DirectionalLights.push_back(light);
				}
				break;
			}
			case sg::LightType::Point:
			{
				if (_LightingState.PointLights.size() < lightCount)
				{
					_LightingState.PointLights.push_back(light);
				}
				break;
			}
			case sg::LightType::Spot:
			{
				if (_LightingState.SpotLights.size() < lightCount)
				{
					_LightingState.SpotLights.push_back(light);
				}
				break;
			}
			default:
				break;
			}
		}

		T lightInfo;
		std::copy(_LightingState.DirectionalLights.begin(), _LightingState.DirectionalLights.end(), lightInfo.DirectionalLights);
		std::copy(_LightingState.PointLights.begin(), _LightingState.PointLights.end(), lightInfo.PointLights);
		std::copy(_LightingState.SpotLights.begin(), _LightingState.SpotLights.end(), lightInfo.SpotLights);

		auto& renderFrame = GetRenderContext().GetActiveFrame();
		_LightingState.LightBuffer = renderFrame.allocate_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(T));
		_LightingState.LightBuffer.Update(lightInfo);
	}

protected:
	RenderContext& _RenderContext;
	VkSampleCountFlagBits _SampleCount = VK_SAMPLE_COUNT_1_BIT;
	std::unordered_map<std::string, ShaderResourceMode> _ResourceModeMap;
	LightingState _LightingState = {}; // contains all the requested render-ready lights for the scene

private:
	std::string _DebugName;
	ShaderSource _VertexShader;
	ShaderSource _FragmentShader;
	DepthStencilState _DepthStencilState = {};
	bool _DisableDepthStencilAttachment = false; // when creating the renderpass, pDepthStencilAttachment will be set to nullptr, which disables depth testing
	// When creating the renderpass, if not None, the resolve of the multisampled depth attachment will be enabled, with this mode, to depth_stencil_resolve_attachment
	VkResolveModeFlagBits _DepthStencilResolveMode = VK_RESOLVE_MODE_NONE;
	std::vector<u32> _InputAttachments = {}; // default to no input attachments
	std::vector<u32> _OutputAttachments = { 0 }; // default to swapchain output attachment
	std::vector<u32> _ColorResolveAttachments = {}; // default to no color resolve attachments
	u32 _DepthStencilResolveAttachment = VK_ATTACHMENT_UNUSED; // default to no depth stencil resolve attachment
};
}	// namespace vge
