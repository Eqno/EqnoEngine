#include <GLSLLibrary/Utils/Math.glsl>
#include <GLSLLibrary/Binding/Fragment/BlinnPhongDeferredProcess.glsl>

void main() {
    ProcessSubpassInput;
    if (FloatEqual(fragPosition.w, pipeline.id) == false) {
        discard;
    }

    float ambientStrength = fragMaterial.x;
    float diffuseStrength = fragMaterial.y;
    float shininessStrength = fragMaterial.z;
    float specularStrength = fragMaterial.w;

    vec4 ambient = vec4(0.);
    vec4 diffuse = vec4(0.);
    vec4 specular = vec4(0.);

    for (int i=0; i<lights.num; ++i) {
        vec3 lightDir = normalize(lights.object[i].pos - fragPosition.xyz);
        vec3 viewDir = normalize(cameraPosition - fragPosition.xyz);
        vec4 light = lights.object[i].color * lights.object[i].intensity;

        ambient += light * ambientStrength;

        #ifdef EnableShadowMap
        vec4 shadowMapPos = lights.object[i].projMatrix * lights.object[i].viewMatrix * vec4(fragPosition.xyz, 1.);
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