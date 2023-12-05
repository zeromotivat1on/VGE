#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInput inputColor; // color output from 1 subpass
layout (input_attachment_index = 1, binding = 1) uniform subpassInput inputDepth; // depth output from 1 subpass

layout (location = 0) out vec4 outColor;

void main() 
{
	// Apply depth visualization.
	const bool depthViz = false;

	if (depthViz)
	{
		const float lowerBound = 0.99f;
		const float upperBound = 1.0f;
		
		const float depth = subpassLoad(inputDepth).r;
		const float depthColorScaled = 1.0f - ((depth - lowerBound) / (upperBound - lowerBound));
		outColor = vec4(subpassLoad(inputColor).rgb * depthColorScaled, 1.0f);
	}
	else
	{
		outColor = subpassLoad(inputColor).rgba;
	}
}
