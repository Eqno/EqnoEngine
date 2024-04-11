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

void main() {
    #ifdef EnableMultiSample
    vec4 fragColor = subpassLoad(inColor, EnableMultiSample);
    vec3 fragNormal = subpassLoad(inNormal, EnableMultiSample).xyz;
    vec3 fragPosition = subpassLoad(inPosition, EnableMultiSample).xyz;
    vec4 fragMaterial = subpassLoad(inMaterial, EnableMultiSample);
    vec3 cameraNormal = subpassLoad(inCameraNormal, EnableMultiSample).xyz;
    vec3 cameraPosition = subpassLoad(inCameraPosition, EnableMultiSample).xyz;
    #else
    vec4 fragColor = subpassLoad(inColor);
    vec3 fragNormal = subpassLoad(inNormal).xyz;
    vec3 fragPosition = subpassLoad(inPosition).xyz;
    vec4 fragMaterial = subpassLoad(inMaterial);
    vec3 cameraNormal = subpassLoad(inCameraNormal).xyz;
    vec3 cameraPosition = subpassLoad(inCameraPosition).xyz;
    #endif

    float ambientStrength = fragMaterial.x;
    float diffuseStrength = fragMaterial.y;
    float shininessStrength = fragMaterial.z;
    float specularStrength = fragMaterial.w;

    vec4 ambient = vec4(0.);
    vec4 diffuse = vec4(0.);
    vec4 specular = vec4(0.);

    for (int i=0; i<lights.num; ++i) {
        vec3 lightDir = normalize(lights.object[i].pos - fragPosition);
        vec3 viewDir = normalize(cameraPosition - fragPosition);
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
    outColor = (ambient + diffuse + specular) * fragColor;
}