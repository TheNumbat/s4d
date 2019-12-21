
#version 400 core

out vec4 out_color;

uniform sampler2DMS depth;
uniform int samples;
uniform vec3 color;

void main() {

    ivec2 coord = ivec2(gl_FragCoord.xy);

    float gx = 0.0f, gy = 0.0f;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
			float dp = texelFetch(depth, coord + ivec2(i,j), gl_SampleID).r;
			int ds = i*i + j*j;
			gx += (i / ds) * dp;
			gy += (j / ds) * dp;
    	}
	}

	float g = sqrt(gx * gx + gy * gy);
	float a = g > 0.5f ? 1.0f : 0.0f;
	out_color = vec4(color * a, a);
}

