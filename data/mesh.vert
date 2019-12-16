
#version 330 core

layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_norm;

uniform mat4 modelview, proj, normal;

smooth out vec3 f_norm;

void main() {
    
    gl_Position = proj * modelview * vec4(v_pos, 1.0f);
    f_norm = (normal * vec4(v_norm, 0.0f)).xyz;
}
