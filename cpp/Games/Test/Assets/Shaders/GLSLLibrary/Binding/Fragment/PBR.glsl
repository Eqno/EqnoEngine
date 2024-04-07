#include <GLSLLibrary/Binding/Fragment/BlinnPhong.glsl>

#ifdef EnableShadowMap
layout(binding = 6) uniform sampler2D roughnessSampler;
layout(binding = 7) uniform sampler2D metallicSampler;
layout(binding = 8) uniform sampler2D normalSampler;
layout(binding = 9) uniform sampler2D AOSampler;
#else
layout(binding = 5) uniform sampler2D roughnessSampler;
layout(binding = 6) uniform sampler2D metallicSampler;
layout(binding = 7) uniform sampler2D normalSampler;
layout(binding = 8) uniform sampler2D AOSampler;
#endif

layout(location = 4) in vec3 fragTangent;