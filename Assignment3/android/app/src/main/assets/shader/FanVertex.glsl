#version 300 es
layout(location = 0) in vec3 aPosition;
layout(location = 1) in float aShade; // Kept to match VAO, but ignored
layout(location = 2) in vec3 aNormal;

uniform mat4 uMVP;
uniform mat4 uModel;
uniform mat3 uNormalMatrix;

out vec3 vFragPos;
out vec3 vNormal;

void main() {
    gl_Position = uMVP * vec4(aPosition, 1.0);
    
    // Transform position to view/world space for lighting
    vFragPos = vec3(uModel * vec4(aPosition, 1.0));
    
    // Apply normal matrix to fix non-uniform scaling issues
    vNormal = uNormalMatrix * aNormal;
}