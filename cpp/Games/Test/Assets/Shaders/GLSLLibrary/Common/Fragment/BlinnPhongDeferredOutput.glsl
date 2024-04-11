layout(binding = 0) uniform CameraData {
    vec3 pos;
    vec3 normal;
} camera;

layout(binding = 1) uniform MaterialData {
    vec4 color;
    float roughness;
    float metallic;
} material;

layout(binding = 2) uniform TransformData {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projMatrix;
} transform;

layout(binding = 3) uniform sampler2D baseColorSampler;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outPosition;
layout(location = 3) out vec4 outMaterial;
layout(location = 4) out vec3 outCameraNormal;
layout(location = 5) out vec3 outCameraPosition;

void main() {
    float ambientStrength = .3;
    float diffuseStrength = .6;
    float shininessStrength = 2.;
    float specularStrength = .8;

    outColor = fragColor * texture(baseColorSampler, fragTexCoord);
    outNormal = fragNormal;
    outPosition = fragPosition;
    outMaterial = vec4(ambientStrength, diffuseStrength, shininessStrength, specularStrength);

    outCameraNormal = camera.normal;
    outCameraPosition = camera.pos;
}