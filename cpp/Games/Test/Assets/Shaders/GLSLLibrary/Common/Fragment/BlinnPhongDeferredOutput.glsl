layout(binding = 3) uniform sampler2D baseColorSampler;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPosition;
layout(location = 3) out vec4 outMaterial;

void main() {
    float ambientStrength = .3;
    float diffuseStrength = .6;
    float shininessStrength = 2.;
    float specularStrength = .8;

    outColor = fragColor * texture(baseColorSampler, fragTexCoord);
    outNormal = fragNormal;
    outPosition = fragPosition;
    outMaterial = vec4(ambientStrength, diffuseStrength, shininessStrength, specularStrength);
}