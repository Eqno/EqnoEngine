#include <GLSLLibrary/Utils/Math.glsl>

float DistributionGGX(const float dnh, const float r) {
    float a = r * r, _dnh = max(0., dnh), dnh_square = _dnh * _dnh;
    return a * a / max(PI * pow(dnh_square * (a * a - 1.) + 1., 2.), EPS);
}

vec3 FresnelSchlick(const float dvh, const vec3 f) {
    return f + (1. - f) * pow(clamp(1. - dvh, 0., 1.), 5.);
}

float GeometrySchlickGGX(float dnx, float k) {
    return dnx / max(dnx * (1. - k) + k, EPS);
}

float GeometrySmith(float dnv, float dnl, float r) {
    return GeometrySchlickGGX(clamp(dnv, 0., 1.), r / 2.)
        * GeometrySchlickGGX(clamp(dnl, 0., 1.), r / 2.);
}

vec3 BRDF(const vec3 normal, const vec3 light, const vec3 view,
	const vec3 color, float roughness, const vec3 fresnel, float metallic)
{
    vec3 _half = normalize(view + light);
    float dvh = dot(view, _half);
    float dnh = dot(normal, _half);
    float dnl = dot(normal, light);
    float dnv = dot(normal, view);
    vec3 _fresnel = mix(fresnel, color, metallic);

    float D = DistributionGGX(dnh, roughness);
    vec3 F = FresnelSchlick(max(dvh, 0.), _fresnel);
    float G = GeometrySmith(dnv, dnl, roughness);

    vec3 specular = D * F * G / max(4. * clamp(dnv, 0., 1.) * clamp(dnl, 0., 1.), EPS);
    vec3 kd = mix(vec3(1.) - F, vec3(0.), metallic);
    return kd * color / PI + specular;
}