#version 300 es
layout(location = 0) in vec3 a_position;
layout(location = 1) in float a_shade;

uniform mat4 uMVP;
out float v_shade;

void main() {
    gl_Position = uMVP * vec4(a_position, 1.0);
    v_shade = a_shade;
}