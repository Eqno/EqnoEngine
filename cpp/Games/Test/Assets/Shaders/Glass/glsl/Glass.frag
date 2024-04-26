#version 450

#include <GLSLLibrary/BRDF/CookTorrance.glsl>
#include <GLSLLibrary/Utils/TBN.glsl>
#include <GLSLLibrary/Binding/DataStructure.glsl>

#ifdef EnableShadowMap
layout(binding = 4) uniform sampler2DShadow shadowMapSamplers[MaxLightNum];
#endif

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) in vec3 fragTangent;

layout(location = 0) out vec4 outColor;

void main() {
}