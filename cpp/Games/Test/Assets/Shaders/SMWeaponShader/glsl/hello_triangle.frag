#version 450

layout(binding = 3) uniform sampler2D baseColorSampler;
layout(binding = 4) uniform sampler2D roughnessSampler;
layout(binding = 5) uniform sampler2D metallicSampler;
layout(binding = 6) uniform sampler2D normalSampler;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = fragColor * texture(baseColorSampler, fragTexCoord);
}