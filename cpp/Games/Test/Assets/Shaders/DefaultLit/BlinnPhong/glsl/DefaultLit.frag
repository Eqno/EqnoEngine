#version 450

#include <GLSLLibrary/Binding/Fragment/BlinnPhong.glsl>

void main() {
    float ambientStrength = .2;
    float diffuseStrength = .4;
    float shininessStrength = 2.;
    float specularStrength = .4;

    vec4 ambient = vec4(0.);
    vec4 diffuse = vec4(0.);
    vec4 specular = vec4(0.);

    for (int i=0; i<lights.num; ++i) {
        vec3 lightDir = normalize(lights.object[i].pos - fragPosition);
        vec3 viewDir = normalize(camera.pos - fragPosition);
        vec4 light = lights.object[i].color * lights.object[i].intensity;

        ambient += light * ambientStrength;
        float diff = max(dot(normalize(fragNormal), lightDir), 0.);
        diffuse += diff * light * diffuseStrength;
        
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normalize(fragNormal), halfwayDir), 0.), shininessStrength);
        specular += spec * light * specularStrength;  
    }

    vec4 texColor = texture(baseColorSampler, fragTexCoord);
    outColor = (ambient + diffuse + specular) * fragColor * texColor;
}