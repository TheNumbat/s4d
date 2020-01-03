
#version 330 core

smooth in vec3 f_col;

uniform float alpha;

out vec4 out_col;

void main() {
    out_col = vec4(f_col, alpha);
}
