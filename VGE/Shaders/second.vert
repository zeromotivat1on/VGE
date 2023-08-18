#version 450

// Array for triangel that fills an entire screen.
vec2 positions[3] = vec2[] 
(
	vec2(3.0, -1.0),
	vec2(-1.0, -1.0),
	vec2(-1.0, 3.0)
);

void main() 
{
	// Generally, vertex shader tells to draw everyting on the screen.
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
