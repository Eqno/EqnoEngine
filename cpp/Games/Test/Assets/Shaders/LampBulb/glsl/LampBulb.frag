#version 450

struct LightData {
    uint type;
    float intensity;
    vec3 pos;
    vec4 color;
    vec3 normal;
};
layout(binding = 3) uniform LightsData {
    uint num;
    LightData object[500];
} lights;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 Illumination = vec4(1.);
    for (int i=0; i<lights.num; ++i) {
        Illumination = lights.object[i].color * lights.object[i].intensity;  
    }
    outColor = Illumination * fragColor;
}