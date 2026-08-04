#version 300 es
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform mat4 uMVP;
uniform mat4 uModel;
uniform mat3 uNormalMatrix;

out vec3 vFragPos;
out vec3 vNormal;
out vec3 vWorldPos; // Used for the procedural checkerboard

void main() {
    gl_Position = uMVP * vec4(aPosition, 1.0);
    vFragPos = vec3(uModel * vec4(aPosition, 1.0));
    vNormal = uNormalMatrix * aNormal;
    vWorldPos = aPosition; // Raw object position creates a static grid
}