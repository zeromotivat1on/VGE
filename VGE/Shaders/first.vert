#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 texCoords;

layout (set = 0, binding = 0) uniform UboViewProjection 
{
	mat4 Projection;
	mat4 View;
} uboViewProjection;

// Left for reference, NOT used.
layout (set = 0, binding = 1) uniform UboModel 
{
	mat4 Model;
} uboModel;

layout (push_constant) uniform PushModel 
{
	mat4 Model;
} pushModel;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragTexCoords;

void main() 
{
	gl_Position = uboViewProjection.Projection * uboViewProjection.View * pushModel.Model * vec4(position, 1.0);
	fragColor = color;
	fragTexCoords = texCoords;
}
