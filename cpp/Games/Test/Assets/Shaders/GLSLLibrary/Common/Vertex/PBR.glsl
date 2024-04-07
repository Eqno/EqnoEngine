#include <GLSLLibrary/Binding/Vertex/PBR.glsl>

void main() {
    fragColor = inColor;
    fragTexCoord = inTexCoord;

    fragNormal = (transform.modelMatrix * vec4(inNormal, 0.)).xyz;
    fragTangent = (transform.modelMatrix * vec4(inTangent, 0.)).xyz;
    fragPosition = (transform.modelMatrix * vec4(inPosition, 1.)).xyz;
    gl_Position = transform.projMatrix * transform.viewMatrix * transform.modelMatrix * vec4(inPosition, 1.);
}