#version 450

layout(binding = 0) uniform CameraData {
    vec3 pos;
    vec3 normal;
} camera;

struct LightData {
    uint type;
    float intensity;
    vec3 pos;
    vec4 color;
    vec3 normal;
};
layout(binding = 3) uniform LightsData {
    uint num;
    LightData object[500];
} lights;

layout(binding = 4) uniform sampler2D baseColorSampler;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

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