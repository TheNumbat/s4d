
#version 330 core

out vec4 out_color;

uniform sampler2DMS tex;
uniform int samples;

void main() {

	ivec2 coord = ivec2(gl_FragCoord.xy);

	vec3 color = vec3(0.0);

	for (int i = 0; i < samples; i++)
		color += texelFetch(tex, coord, i).xyz;

	color /= float(samples);

	out_color = vec4(color, 1.0f);
}

