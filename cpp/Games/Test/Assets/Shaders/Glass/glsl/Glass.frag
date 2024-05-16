#version 450

#include <GLSLLibrary/BRDF/CookTorrance.glsl>
#include <GLSLLibrary/Utils/TBN.glsl>
#include <GLSLLibrary/Binding/DataStructure.glsl>

#ifdef EnableShadowMap
layout(binding = 4) uniform sampler2DShadow shadowMapSamplers[MaxLightNum];
layout(binding = 5) uniform sampler2D baseColorSampler;
#else
layout(binding = 4) uniform sampler2D baseColorSampler;
#endif

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    discard;

    float ambientStrength = .3;
    float diffuseStrength = .6;
    float shininessStrength = 2.;
    float specularStrength = .8;

    vec4 ambient = vec4(0.);
    vec4 diffuse = vec4(0.);
    vec4 specular = vec4(0.);

    for (int i=0; i<lights.num; ++i) {
        vec3 lightDir = normalize(lights.object[i].pos - fragPosition);
        vec3 viewDir = normalize(camera.pos - fragPosition);
        vec4 light = lights.object[i].color * lights.object[i].intensity;

        ambient += light * ambientStrength;

        #ifdef EnableShadowMap
        vec4 shadowMapPos = lights.object[i].projMatrix * lights.object[i].viewMatrix * vec4(fragPosition, 1.);
        shadowMapPos /= shadowMapPos.w;

        if (shadowMapPos.x > -1.0 && shadowMapPos.x < 1.0 && shadowMapPos.y > -1.0 && shadowMapPos.y < 1.0  && shadowMapPos.z > -1.0 && shadowMapPos.z < 1.0) {
            shadowMapPos.x = 0.5 * shadowMapPos.x + 0.5;
            shadowMapPos.y = 0.5 * shadowMapPos.y + 0.5;

            float shadowMapZ = texture(shadowMapSamplers[lights.object[i].id], shadowMapPos.xyz);
            if (shadowMapZ >= shadowMapPos.z) {
        #endif

                float diff = max(dot(normalize(fragNormal), lightDir), 0.);
                diffuse += diff * light * diffuseStrength;
                
                vec3 halfwayDir = normalize(lightDir + viewDir);
                float spec = pow(max(dot(normalize(fragNormal), halfwayDir), 0.), shininessStrength);
                specular += spec * light * specularStrength;

            #ifdef EnableShadowMap
            }
        }
        #endif
    }
    vec4 texColor = texture(baseColorSampler, fragTexCoord);
    outColor = (ambient + diffuse + specular) * fragColor * texColor;
}