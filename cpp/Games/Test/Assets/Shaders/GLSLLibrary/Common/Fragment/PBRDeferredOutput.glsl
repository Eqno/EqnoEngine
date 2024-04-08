layout(binding = 3) uniform sampler2D baseColorSampler;
layout(binding = 4) uniform sampler2D roughnessSampler;
layout(binding = 5) uniform sampler2D metallicSampler;
layout(binding = 6) uniform sampler2D normalSampler;
layout(binding = 7) uniform sampler2D AOSampler;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) in vec3 fragTangent;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPosition;
layout(location = 3) out vec4 outMaterial;

void main() {
    outColor = fragColor * texture(baseColorSampler, fragTexCoord);
    vec3 texNormal = normalize(texture(normalSampler, fragTexCoord).xyz);
    outNormal = TBN(fragPosition, fragTexCoord, fragNormal, texNormal);
    outPosition = fragPosition;

    float texRoughness = texture(roughnessSampler, fragTexCoord)[0];
    float texMetallic = texture(metallicSampler, fragTexCoord)[0];
    float texAO = texture(AOSampler, fragTexCoord)[0];
    outMaterial = vec4(texRoughness, texMetallic, texAO, 0.);
}