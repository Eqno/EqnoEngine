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