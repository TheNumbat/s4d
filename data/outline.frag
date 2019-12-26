
#version 400 core

out vec4 out_color;

uniform sampler2D depth;
uniform vec3 color;
uniform vec2 i_screen_size;

void main() {

    ivec2 coord = ivec2(gl_FragCoord.xy);
	float o = 1.0f / texture(depth, coord * i_screen_size).r;

	float diff = 0.0f;
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			float d = 1.0f / texture(depth, (coord + ivec2(i,j)) * i_screen_size).r;
			diff = max(diff, abs(o - d));
		}
	}

	float a = isinf(diff) ? 1.0f : 0.0f;
	out_color = vec4(color * a, a);
}

