#include <GLSLLibrary/Binding/DataStructureDeferredProcess.glsl>

#ifdef EnableMultiSample
layout(input_attachment_index = 0, binding = 1) uniform subpassInputMS inColor;
layout(input_attachment_index = 1, binding = 2) uniform subpassInputMS inNormal;
layout(input_attachment_index = 2, binding = 3) uniform subpassInputMS inPosition;
layout(input_attachment_index = 3, binding = 4) uniform subpassInputMS inMaterial;
layout(input_attachment_index = 4, binding = 5) uniform subpassInputMS inCameraNormal;
layout(input_attachment_index = 5, binding = 6) uniform subpassInputMS inCameraPosition;
#else
layout(input_attachment_index = 0, binding = 1) uniform subpassInput inColor;
layout(input_attachment_index = 1, binding = 2) uniform subpassInput inNormal;
layout(input_attachment_index = 2, binding = 3) uniform subpassInput inPosition;
layout(input_attachment_index = 3, binding = 4) uniform subpassInput inMaterial;
layout(input_attachment_index = 4, binding = 5) uniform subpassInput inCameraNormal;
layout(input_attachment_index = 5, binding = 6) uniform subpassInput inCameraPosition;
#endif

#ifdef EnableShadowMap
layout(binding = 7) uniform sampler2DShadow shadowMapSamplers[MaxLightNum];
#endif

layout(location = 0) out vec4 outColor;

#ifdef EnableMultiSample
#define ProcessSubpassInput \
vec4 fragColor = vec4(0.); \
vec3 fragNormal = vec3(0.); \
vec3 fragPosition = vec3(0.); \
vec4 fragMaterial = vec4(0.); \
vec3 cameraNormal = vec3(0.); \
vec3 cameraPosition = vec3(0.); \
for (int i=0; i<EnableMultiSample; i++) { \
    fragColor += subpassLoad(inColor, i); \
    fragNormal += subpassLoad(inNormal, i).xyz; \
    fragPosition += subpassLoad(inPosition, i).xyz; \
    fragMaterial += subpassLoad(inMaterial, i); \
    cameraNormal += subpassLoad(inCameraNormal, i).xyz; \
    cameraPosition += subpassLoad(inCameraPosition, i).xyz; \
} \
fragColor /= EnableMultiSample; \
fragNormal /= EnableMultiSample; \
fragPosition /= EnableMultiSample; \
fragMaterial /= EnableMultiSample; \
cameraNormal /= EnableMultiSample; \
cameraPosition /= EnableMultiSample
#else
#define ProcessSubpassInput \
vec4 fragColor = subpassLoad(inColor); \
vec3 fragNormal = subpassLoad(inNormal).xyz; \
vec3 fragPosition = subpassLoad(inPosition).xyz; \
vec4 fragMaterial = subpassLoad(inMaterial); \
vec3 cameraNormal = subpassLoad(inCameraNormal).xyz; \
vec3 cameraPosition = subpassLoad(inCameraPosition).xyz
#endif