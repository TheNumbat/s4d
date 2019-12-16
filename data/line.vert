
#version 330 core

layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_col;

uniform mat4 proj_view;
smooth out vec3 f_col;

void main() {
    gl_Position = proj_view * vec4(v_pos, 1.0f);
    f_col = v_col;
}
