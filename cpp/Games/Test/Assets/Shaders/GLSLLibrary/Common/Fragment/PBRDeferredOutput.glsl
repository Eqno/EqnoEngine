#include <GLSLLibrary/Utils/TBN.glsl>
#include <GLSLLibrary/Binding/Fragment/PBRDeferredOutput.glsl>

void main() {
    outColor = fragColor * texture(baseColorSampler, fragTexCoord);
    vec3 texNormal = normalize(texture(normalSampler, fragTexCoord).xyz);
    outNormal = TBN(fragPosition, fragTexCoord, fragNormal, texNormal);
    outPosition = fragPosition;

    float texRoughness = texture(roughnessSampler, fragTexCoord)[0];
    float texMetallic = texture(metallicSampler, fragTexCoord)[0];
    float texAO = texture(AOSampler, fragTexCoord)[0];
    outMaterial = vec4(texRoughness, texMetallic, texAO, -1.);
}