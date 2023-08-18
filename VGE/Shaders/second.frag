#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInput inputColor; // color output from 1 subpass
layout (input_attachment_index = 1, binding = 1) uniform subpassInput inputDepth; // depth output from 2 subpass

layout (location = 0) out vec4 outColor;

void main() 
{
	outColor = subpassLoad(inputColor).rgba;
	outColor.g = 0.0f; // remove green channel entirely
}
