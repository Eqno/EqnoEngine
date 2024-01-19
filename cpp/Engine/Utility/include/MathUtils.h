#pragma once

#include <cmath>
#include <glm/vec3.hpp>

namespace MathUtils {
using namespace glm;
using namespace std;

vec3 rotateX(const vec3 p, const float a) {
  float c = cos(a), s = sin(a);
  return vec3(p.x, c * p.y - s * p.z, s * p.y + c * p.z);
}
vec3 rotateY(const vec3 p, const float a) {
  float c = cos(a), s = sin(a);
  return vec3(c * p.x + s * p.z, p.y, -s * p.x + c * p.z);
}
vec3 rotateZ(const vec3 p, const float a) {
  float c = cos(a), s = sin(a);
  return vec3(c * p.x - s * p.y, s * p.x + c * p.y, p.z);
}
vec3 rotate(vec3 point, const vec3 angles) {
  point = rotateX(point, angles.x);
  point = rotateY(point, angles.y);
  point = rotateZ(point, angles.z);
  return point;
}
}  // namespace MathUtils