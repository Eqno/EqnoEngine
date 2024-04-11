layout(location = 0) in vec4 inColor;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inPosition;
layout(location = 3) in vec4 inMaterial;
layout(location = 4) in vec3 inCameraNormal;
layout(location = 5) in vec3 inCameraPosition;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragPosition;
layout(location = 3) out vec4 fragMaterial;
layout(location = 4) out vec3 outCameraNormal;
layout(location = 5) out vec3 outCameraPosition;

void main() {
    fragColor = inColor;
    fragNormal = inNormal;
    fragPosition = inPosition;
    fragMaterial = inMaterial;
    outCameraNormal = inCameraNormal;
    outCameraPosition = inCameraPosition;
    gl_Position = vec4(0.);
}