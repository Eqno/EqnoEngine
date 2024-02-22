#version 450

layout(binding = 1) uniform MaterialData {
    vec4 color;
    float roughness;
    float metallic;
} material;

layout(binding = 4) uniform sampler2DShadow shadowMapSamplers[MaxLightNum];
layout(binding = 5) uniform sampler2D baseColorSampler;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = material.color * fragColor * texture(baseColorSampler, fragTexCoord);
    outColor = pow(outColor, vec4(.45, .45, .45, 1.));
}