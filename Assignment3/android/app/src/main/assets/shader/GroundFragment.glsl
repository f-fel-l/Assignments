#version 300 es
precision mediump float;

in vec3 vFragPos;
in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 uLightPos;
uniform vec3 uViewPos;

out vec4 FragColor;

// Procedural pattern math from the prompt
float checker(vec2 worldxz, float tileSize) {
    vec2 cell = floor(worldxz / tileSize);
    float parity = mod(cell.x + cell.y, 2.0);
    return parity; // 0.0 or 1.0
}

void main() {
    // 1. Procedural Checkerboard Color
    float parity = checker(vWorldPos.xz, 2.0);
    vec3 color1 = vec3(0.15, 0.15, 0.15); // Dark Gray
    vec3 color2 = vec3(0.85, 0.85, 0.85); // Light Gray
    vec3 groundColor = mix(color1, color2, parity);

    // 2. Phong Lighting (Exact same math as the fan)
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * groundColor;

    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(uLightPos - vFragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * groundColor;

    float specularStrength = 0.3;
    vec3 viewDir = normalize(uViewPos - vFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}