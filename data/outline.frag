
#version 400 core

in vec2 f_uv;

out vec4 out_color;

uniform sampler2D depth;
uniform vec3 color;
uniform vec2 i_tex_size;

void main() {

    float gx = 0.0f, gy = 0.0f;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
			float dp = texture(depth, f_uv + vec2(i,j) * i_tex_size).r;
			int ds = i*i + j*j;
			gx += (i / ds) * dp;
			gy += (j / ds) * dp;
    	}
	}

	float g = sqrt(gx * gx + gy * gy);
	float a = g > 0.5f ? 1.0f : 0.0f;

	out_color = vec4(color * a, a);
	// out_color = vec4(vec3(texture(depth, f_uv).r), 1.0f);
}

