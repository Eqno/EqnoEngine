#version 450

layout(binding = 2) uniform TransformData {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projMatrix;
} transform;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec4 fragColor;
layout(location = 2) out vec3 fragNormal;

void main() {
    fragColor = inColor;

    fragNormal = (transform.modelMatrix * vec4(inNormal, 0.)).xyz;
    fragPosition = (transform.modelMatrix * vec4(inPosition, 1.)).xyz;
    gl_Position = transform.projMatrix * transform.viewMatrix * transform.modelMatrix * vec4(inPosition, 1.);
}