
#version 330 core

in vec2 f_uv;

out vec4 out_color;

uniform sampler2DMS tex;
uniform int samples;
uniform vec2 tex_size;

void main() {

	ivec2 coord = ivec2(f_uv * tex_size);

	vec3 color = vec3(0.0);

	for (int i = 0; i < samples; i++)
		color += texelFetch(tex, coord, i).xyz;

	color /= float(samples);

	out_color = vec4(color, 1.0f);
}

