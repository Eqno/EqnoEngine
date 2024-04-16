#include <GLSLLibrary/Binding/Fragment/BlinnPhongDeferredOutput.glsl>

void main() {
    float ambientStrength = .3;
    float diffuseStrength = .6;
    float shininessStrength = 2.;
    float specularStrength = .8;

    outColor = fragColor * texture(baseColorSampler, fragTexCoord);
    outNormal = fragNormal;
    outPosition = vec4(fragPosition, pipeline.id);
    outMaterial = vec4(ambientStrength, diffuseStrength, shininessStrength, specularStrength);

    outCameraNormal = camera.normal;
    outCameraPosition = camera.pos;
}