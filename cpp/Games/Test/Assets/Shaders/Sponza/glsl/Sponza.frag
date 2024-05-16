#version 450

#include <GLSLLibrary/BRDF/CookTorrance.glsl>
#include <GLSLLibrary/Utils/TBN.glsl>
#include <GLSLLibrary/Binding/DataStructure.glsl>

#ifdef EnableShadowMap
layout(binding = 4) uniform sampler2DShadow shadowMapSamplers[MaxLightNum];
layout(binding = 5) uniform sampler2D baseColorSampler;
layout(binding = 6) uniform sampler2D roughnessSampler;
layout(binding = 7) uniform sampler2D metallicSampler;
layout(binding = 8) uniform sampler2D normalSampler;
#else
layout(binding = 4) uniform sampler2D baseColorSampler;
layout(binding = 5) uniform sampler2D roughnessSampler;
layout(binding = 6) uniform sampler2D metallicSampler;
layout(binding = 7) uniform sampler2D normalSampler;
#endif

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 texBaseColor = texture(baseColorSampler, fragTexCoord);
    float texRoughness = texture(roughnessSampler, fragTexCoord)[0];
    float texMetallic = texture(metallicSampler, fragTexCoord)[0];
    vec3 texNormal = normalize(texture(normalSampler, fragTexCoord).xyz);

    float DiffuseStrength = .5;
    float SpecularStength = 5.;
    outColor = DiffuseStrength * texBaseColor / PI;

    for (int i=0; i<lights.num; ++i) {

        vec3 lightPos = lights.object[i].pos;
        vec4 lightColor = lights.object[i].color;
        vec3 lightNormal = normalize(lights.object[i].normal);
        float lightIntensity = lights.object[i].intensity;

        vec3 lightDir = normalize(lights.object[i].pos - fragPosition);
        vec3 viewDir = normalize(camera.pos - fragPosition);

        #ifdef EnableShadowMap
        vec4 shadowMapPos = lights.object[i].projMatrix * lights.object[i].viewMatrix * vec4(fragPosition, 1.);
        shadowMapPos /= shadowMapPos.w;

        if (shadowMapPos.x > -1.0 && shadowMapPos.x < 1.0 && shadowMapPos.y > -1.0 && shadowMapPos.y < 1.0  && shadowMapPos.z > -1.0 && shadowMapPos.z < 1.0) {
            shadowMapPos.x = 0.5 * shadowMapPos.x + 0.5;
            shadowMapPos.y = 0.5 * shadowMapPos.y + 0.5;

            float shadowMapZ = texture(shadowMapSamplers[lights.object[i].id], shadowMapPos.xyz);
            if (shadowMapZ >= shadowMapPos.z) {
        #endif

        if (lights.object[i].type == 1) {
            vec3 brdf = BRDF(TBN(fragPosition, fragTexCoord, fragNormal, texNormal), lightNormal, viewDir, texBaseColor.rgb, texRoughness, vec3(0.1), texMetallic);
            outColor += SpecularStength * vec4(brdf, 1.) * lightColor * lightIntensity;
        }
        if (lights.object[i].type == 3) {
            vec3 brdf = BRDF(TBN(fragPosition, fragTexCoord, fragNormal, texNormal), lightDir, viewDir, texBaseColor.rgb, texRoughness, vec3(0.1), texMetallic);
            outColor += SpecularStength * vec4(brdf, 1.) * lightColor * lightIntensity;
        }

        #ifdef EnableShadowMap
            }
        }
        #endif
    }
    outColor = pow(outColor, vec4(.45, .45, .45, 1.));
}