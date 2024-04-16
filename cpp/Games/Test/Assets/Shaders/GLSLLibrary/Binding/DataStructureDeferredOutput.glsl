layout(binding = 0) uniform PipelineData {
    int id;
} pipeline;

layout(binding = 1) uniform CameraData {
    vec3 pos;
    vec3 normal;
} camera;

layout(binding = 2) uniform MaterialData {
    vec4 color;
    float roughness;
    float metallic;
} material;

layout(binding = 3) uniform TransformData {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projMatrix;
} transform;