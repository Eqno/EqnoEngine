#version 450

layout(binding = 1) uniform MaterialData {
    vec4 color;
    float roughness;
    float metallic;
} material;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = material.color * fragColor;
}