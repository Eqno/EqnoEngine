#include <GLSLLibrary/BRDF/CookTorrance.glsl>
#include <GLSLLibrary/Binding/Fragment/PBRDeferredProcess.glsl>

void main() {
    ProcessSubpassInput;
    if (FloatEqual(fragPosition.w, pipeline.id) == false) {
        discard;
    }

    float texRoughness = fragMaterial.x;
    float texMetallic = fragMaterial.y;
    float texAO = fragMaterial.z;
    
    float DiffuseStrength = .5;
    float SpecularStength = 5.;
    outColor = DiffuseStrength * fragColor / PI * texAO;

    for (int i=0; i<lights.num; ++i) {

        vec3 lightPos = lights.object[i].pos;
        vec4 lightColor = lights.object[i].color;
        vec3 lightNormal = normalize(lights.object[i].normal);
        float lightIntensity = lights.object[i].intensity;

        vec3 lightDir = normalize(lights.object[i].pos - fragPosition.xyz);
        vec3 viewDir = normalize(cameraPosition - fragPosition.xyz);

        if (lights.object[i].type == 1) {
            vec3 brdf = BRDF(fragNormal, lightNormal, viewDir, fragColor.rgb, texRoughness, vec3(0.1), texMetallic);
            outColor += SpecularStength * vec4(brdf, 1.) * lightColor * lightIntensity;
        }
        if (lights.object[i].type == 3) {
            vec3 brdf = BRDF(fragNormal, lightDir, viewDir, fragColor.rgb, texRoughness, vec3(0.1), texMetallic);
            outColor += SpecularStength * vec4(brdf, 1.) * lightColor * lightIntensity;
        }
    }
    outColor = pow(outColor, vec4(.45, .45, .45, 1.));
}