#include <GLSLLibrary/Binding/Fragment/BlinnPhongDeferredOutput.glsl>

layout(binding = 5) uniform sampler2D roughnessSampler;
layout(binding = 6) uniform sampler2D metallicSampler;
layout(binding = 7) uniform sampler2D normalSampler;
layout(binding = 8) uniform sampler2D AOSampler;

layout(location = 4) in vec3 fragTangent;