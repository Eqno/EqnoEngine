layout(location = 0) in vec4 inColor;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec4 inPosition;
layout(location = 3) in vec4 inMaterial;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 fragNormal;
layout(location = 2) out vec4 fragPosition;
layout(location = 3) out vec4 fragMaterial;

void main() {
    fragColor = inColor;
    fragNormal = inNormal;
    fragPosition = inPosition;
    fragMaterial = inMaterial;
    gl_Position = vec4(0.);
}