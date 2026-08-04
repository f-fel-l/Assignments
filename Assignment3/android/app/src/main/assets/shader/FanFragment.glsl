#version 300 es
precision mediump float;

in vec3 vFragPos;
in vec3 vNormal;

uniform vec3 PARTCOLOR;
uniform vec3 uLightPos;
uniform vec3 uViewPos;

out vec4 FragColor;

void main() {
    // 1. Ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * PARTCOLOR;

    // 2. Diffuse
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(uLightPos - vFragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * PARTCOLOR;

    // 3. Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(uViewPos - vFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    // 32.0 is the shininess factor
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0); 

    // Combine
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}