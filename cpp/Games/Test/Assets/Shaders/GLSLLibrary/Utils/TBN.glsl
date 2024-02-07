vec3 TBN(vec3 fragPosition, vec2 fragTexCoord, vec3 fragNormal, vec3 texNormal) {
    vec3 posDx  = dFdx(fragPosition);
    vec3 posDy  = dFdy(fragPosition);
    vec3 stx    = dFdx(vec3(fragTexCoord, 0.0));
    vec3 sty    = dFdy(vec3(fragTexCoord, 0.0));
    vec3 T      = (sty.t * posDx - stx.t * posDy) / (stx.s * sty.t - sty.s * stx.t);
    vec3 N      = normalize(fragNormal);
    T           = normalize(T - N * dot(N, T));
    vec3 B      = normalize(cross(N, T));
    mat3 TBN    = mat3(T, B, N);
    return normalize(TBN * normalize(2.0 * texNormal - 1.0));
}
