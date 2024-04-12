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