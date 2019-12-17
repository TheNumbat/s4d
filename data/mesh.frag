
#version 330 core

uniform vec3 color;
uniform bool solid;

out vec4 out_col;

smooth in vec3 f_norm;

void main() {

    if(solid) {
        out_col = vec4(color, 1.0f);
        return;
    }

    float ndotl = max(normalize(f_norm).z, 0.0f);
    float light = clamp(0.2f + ndotl, 0.0f, 1.0f);

    out_col = vec4(light * color, 1.0f);
}
