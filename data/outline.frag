
#version 330 core

in vec2 f_uv;

out vec4 out_color;

uniform sampler2DMS depth;
uniform int samples;
uniform vec2 tex_size;

void main() {

	out_color = vec4(0.5f, 0.0f, 0.0f, 0.5f);
}

