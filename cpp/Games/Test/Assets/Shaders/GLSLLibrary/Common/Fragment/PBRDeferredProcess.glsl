struct LightData {
    int id;
    uint type;
    float intensity;
    vec3 pos;
    vec4 color;
    vec3 normal;
    mat4 viewMatrix;
    mat4 projMatrix;
};

layout(binding = 0) uniform LightsData {
    uint num;
    LightData object[MaxLightNum];
} lights;

#ifdef EnableShadowMap
layout(binding = 1) uniform sampler2DShadow shadowMapSamplers[MaxLightNum];
#endif

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec4 fragNormal;
layout(location = 2) in vec4 fragPosition;
layout(location = 3) in vec4 fragMaterial;

void main() {
    float texRoughness = fragMaterial[0];
    float texMetallic = fragMaterial[1];
    float texAO = fragMaterial[2];    

    float DiffuseStrength = .5;
    float SpecularStength = 5.;
    outColor = DiffuseStrength * fragColor / PI * texAO;

    for (int i=0; i<lights.num; ++i) {

        vec3 lightPos = lights.object[i].pos;
        vec4 lightColor = lights.object[i].color;
        vec3 lightNormal = normalize(lights.object[i].normal);
        float lightIntensity = lights.object[i].intensity;

        vec3 lightDir = normalize(lights.object[i].pos - fragPosition);
        vec3 viewDir = normalize(camera.pos - fragPosition);

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