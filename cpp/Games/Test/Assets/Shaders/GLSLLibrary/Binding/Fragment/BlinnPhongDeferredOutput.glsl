#include <GLSLLibrary/Binding/DataStructureDeferredOutput.glsl>

layout(binding = 3) uniform sampler2D baseColorSampler;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outPosition;
layout(location = 3) out vec4 outMaterial;
layout(location = 4) out vec3 outCameraNormal;
layout(location = 5) out vec3 outCameraPosition;