#version 450

#include <GLSLLibrary/Binding/Fragment/BlinnPhong.glsl>

void main() {
    outColor = material.color * fragColor * texture(baseColorSampler, fragTexCoord);
    outColor = pow(outColor, vec4(.45, .45, .45, 1.));
}