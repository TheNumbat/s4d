
#version 330 core

uniform vec3 color;
uniform bool solid, write_id;
uniform int id;

layout (location = 0) out vec4 out_col;
layout (location = 1) out vec4 out_id;

smooth in vec3 f_norm;

void main() {

    if(write_id) {
        out_id = vec4((id & 0xff) / 255.0f, ((id >> 8) & 0xff) / 255.0f, ((id >> 16) & 0xff) / 255.0f, 1.0f);
    }

    if(solid) {
        out_col = vec4(color, 1.0f);
    } else {
        float ndotl = max(normalize(f_norm).z, 0.0f);
        float light = clamp(0.2f + ndotl, 0.0f, 1.0f);

        out_col = vec4(light * color, 1.0f);
    }
}
