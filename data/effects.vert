
#version 330 core

layout (location = 0) in vec2 v_pos;

uniform vec2 bounds[4];

void main() {
	gl_Position = vec4(bounds[gl_VertexID], 0.0f, 1.0f);
}
