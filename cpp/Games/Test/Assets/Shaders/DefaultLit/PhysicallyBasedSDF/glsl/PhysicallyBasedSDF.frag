#version 450

#include <GLSLLibrary/BRDF/CookTorrance.glsl>
#include <GLSLLibrary/Utils/TBN.glsl>
#include <GLSLLibrary/Binding/Fragment/PBR.glsl>

void main() {
    vec4 texBaseColor = texture(baseColorSampler, fragTexCoord);
    float texRoughness = texture(roughnessSampler, fragTexCoord)[0];
    float texMetallic = texture(metallicSampler, fragTexCoord)[0];
    vec3 texNormal = normalize(texture(normalSampler, fragTexCoord).xyz);
    float texAO = texture(AOSampler, fragTexCoord)[0];

    float DiffuseStrength = .5;
    float SpecularStength = 5.;
    outColor = DiffuseStrength * texBaseColor / PI * texAO;

    for (int i=0; i<lights.num; ++i) {

        vec3 lightPos = lights.object[i].pos;
        vec4 lightColor = lights.object[i].color;
        vec3 lightNormal = normalize(lights.object[i].normal);
        float lightIntensity = lights.object[i].intensity;

        vec3 lightDir = normalize(lights.object[i].pos - fragPosition);
        vec3 viewDir = normalize(camera.pos - fragPosition);

        if (lights.object[i].type == 1) {
            vec3 brdf = BRDF(TBN(fragPosition, fragTexCoord, fragNormal, texNormal), lightNormal, viewDir, texBaseColor.rgb, texRoughness, vec3(0.1), texMetallic);
            outColor += SpecularStength * vec4(brdf, 1.) * lightColor * lightIntensity;
        }
        if (lights.object[i].type == 3) {
            vec3 brdf = BRDF(TBN(fragPosition, fragTexCoord, fragNormal, texNormal), lightDir, viewDir, texBaseColor.rgb, texRoughness, vec3(0.1), texMetallic);
            outColor += SpecularStength * vec4(brdf, 1.) * lightColor * lightIntensity;
        }
    }
    outColor = pow(outColor, vec4(.45, .45, .45, 1.));
}