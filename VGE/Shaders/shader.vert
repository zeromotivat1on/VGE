#version 450

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Color;

layout (binding = 0) uniform UboViewProjection {
	mat4 Projection;
	mat4 View;
} uboViewProjection;

layout (binding = 1) uniform UboModel {
	mat4 Model;
} uboModel;

layout (location = 0) out vec3 fragColor;

void main() {
	gl_Position = uboViewProjection.Projection * uboViewProjection.View * uboModel.Model * vec4(Position, 1.0);
	fragColor = Color;
}
