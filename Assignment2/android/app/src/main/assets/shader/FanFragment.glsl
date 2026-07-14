#version 300 es
precision mediump float;

in float v_shade;
uniform vec3 PARTCOLOR;

out vec4 outColor;

void main() {
    outColor = vec4(PARTCOLOR * v_shade, 1.0);
}