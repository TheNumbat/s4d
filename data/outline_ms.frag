
#version 400 core

out vec4 out_color;

uniform sampler2DMS depth;
uniform vec3 color;

void main() {

    ivec2 coord = ivec2(gl_FragCoord.xy);
	float o = texelFetch(depth, coord, gl_SampleID).r;

	float diff = 0.0f;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
			vec4 d = texelFetch(depth, coord + ivec2(i,j), gl_SampleID);
			diff = max(diff, abs(o - d.r * d.a));
    	}
	}

	float a = diff > 0.1f ? 1.0f : 0.0f;
	out_color = vec4(color * a, a);
}

