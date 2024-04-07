layout(binding = 0) uniform CameraData {
    vec3 pos;
    vec3 normal;
} camera;

layout(binding = 1) uniform MaterialData {
    vec4 color;
    float roughness;
    float metallic;
} material;

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

layout(binding = 3) uniform LightsData {
    uint num;
    LightData object[MaxLightNum];
} lights;