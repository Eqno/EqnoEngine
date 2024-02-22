#version 450

struct LightData {
    int id;
    uint type;
    float intensity;
    vec3 pos;
    vec4 color;
    vec3 normal;
    mat4 viewMatrix;
    mat4 projMatrix;
};
layout(binding = 3) uniform LightsData {
    uint num;
    LightData object[500];
} lights;

layout(binding = 4) uniform sampler2DShadow shadowMapSamplers[MaxLightNum];

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = 3. * lights.object[0].color * lights.object[0].intensity * fragColor;
}