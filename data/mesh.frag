
#version 330 core

uniform vec3 color;

out vec4 out_col;

void main() {
    out_col = vec4(color, 1.0f);
}
