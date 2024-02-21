#version 450

layout(binding = 0) uniform TransformData {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projMatrix;
} transform;

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
layout(binding = 1) uniform LightsData {
    uint num;
    LightData object[500];
} lights;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec2 inTexCoord;

void main() {
    for (int i=0; i<lights.num; i++) {
        gl_Position = lights.object[i].projMatrix * lights.object[i].viewMatrix * transform.modelMatrix * vec4(inPosition, 1.);
    }
}