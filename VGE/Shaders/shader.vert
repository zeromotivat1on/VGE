#version 450

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Color;

layout (binding = 0) uniform MVP {
	mat4 Projection;
	mat4 View;
	mat4 Model;
} mvp;

layout (location = 0) out vec3 fragColor;

void main() {
	gl_Position = mvp.Projection * mvp.View * mvp.Model * vec4(Position, 1.0);
	fragColor = Color;
}