#version 450

layout(binding = 0) uniform TransformData {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projMatrix;
} transform;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec2 inTexCoord;

void main() {
    gl_Position = transform.projMatrix * transform.viewMatrix * transform.modelMatrix * vec4(inPosition, 1.);
}