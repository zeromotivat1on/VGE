#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInput inputColor; // color output from 1 subpass
layout (input_attachment_index = 1, binding = 1) uniform subpassInput inputDepth; // depth output from 1 subpass

layout (location = 0) out vec4 outColor;

void main() 
{
	int winXHalf = 800 / 2;
	if(gl_FragCoord.x > winXHalf)
	{
		const float lowerBound = 0.98f;
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
