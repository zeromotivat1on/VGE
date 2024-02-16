#include "Subpass.h"

namespace vge
{
const std::vector<std::string> GLightTypeDefinitions = {
	"DIRECTIONAL_LIGHT " + std::to_string(static_cast<float>(sg::LightType::Directional)),
	"POINT_LIGHT " + std::to_string(static_cast<float>(sg::LightType::Point)),
	"SPOT_LIGHT " + std::to_string(static_cast<float>(sg::LightType::Spot)) };

glm::mat4 VulkanStyleProjection(const glm::mat4& proj)
{
	// Flip Y in clipspace. X = -1, Y = -1 is topLeft in Vulkan.
	glm::mat4 mat = proj;
	mat[1][1] *= -1;

	return mat;
}
}	// namespace vge

vge::Subpass::Subpass(RenderContext& renderContext, ShaderSource&& vertexSource, ShaderSource&& fragmentSource) 
	: _RenderContext(renderContext), _VertexShader(std::move(vertexSource)), _FragmentShader(std::move(fragmentSource))
{
}

void vge::Subpass::UpdateRenderTargetAttachments(RenderTarget& renderTarget)
{
	renderTarget.set_input_attachments(_InputAttachments);
	renderTarget.set_output_attachments(_OutputAttachments);
}
