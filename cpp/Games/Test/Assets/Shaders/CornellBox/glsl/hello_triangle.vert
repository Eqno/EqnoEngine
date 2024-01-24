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
    fragTexCoord = inTexCoord;
}