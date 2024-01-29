#version 450

layout(binding = 0) uniform TransformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} transform;

layout(binding = 1) uniform MaterialBufferObject {
    vec4 color;
    float roughness;
    float metallic;
} material;

layout(binding = 2) uniform LightBufferObject {
    uint type;
    float intensity;
    vec3 pos;
    vec4 color;
    vec3 normal;
} lights[500];

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec2 inTexCoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = transform.proj * transform.view * transform.model * vec4(inPosition, 1.0);
    fragColor =  material.color * inColor;
    if (lights[0].type == 3 && lights[0].intensity > 0.9) {
        fragColor *= lights[0].color;
    }
    fragTexCoord = inTexCoord;
}