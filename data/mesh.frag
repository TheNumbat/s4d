
#version 330 core

uniform vec3 color;

out vec4 out_col;

smooth in vec3 f_norm;

void main() {
    float ndotl = max(normalize(f_norm).z, 0.0f);
    float light = clamp(0.2f + ndotl, 0.0f, 1.0f);

    out_col = vec4(light * color, 1.0f);
}
