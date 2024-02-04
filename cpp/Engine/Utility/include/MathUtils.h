#pragma once

#include <assimp/types.h>

#include <cmath>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace MathUtils {
using namespace glm;
using namespace std;

inline glm::vec4 AiColor4D2GlmVec4(const aiColor4D& vec) {
  return {vec.r, vec.g, vec.b, vec.a};
}
inline glm::vec4 AiColor3D2GlmVec4(const aiColor3D& vec) {
  return {vec.r, vec.g, vec.b, 1};
}
inline glm::vec3 AiVector3D2GlmVec3(const aiVector3D& vec) {
  return {vec.x, vec.y, vec.z};
}
inline glm::vec2 AiVector2D2GlmVec2(const aiVector2D& vec) {
  return {vec.x, vec.y};
}

#define RotateFunctions(LibType)                                \
  inline LibType rotateX(const LibType& p, const float a) {     \
    float c = cos(a), s = sin(a);                               \
    return LibType(p.x, c * p.y - s * p.z, s * p.y + c * p.z);  \
  }                                                             \
  inline LibType rotateY(const LibType& p, const float a) {     \
    float c = cos(a), s = sin(a);                               \
    return LibType(c * p.x + s * p.z, p.y, -s * p.x + c * p.z); \
  }                                                             \
  inline LibType rotateZ(const LibType& p, const float a) {     \
    float c = cos(a), s = sin(a);                               \
    return LibType(c * p.x - s * p.y, s * p.x + c * p.y, p.z);  \
  }                                                             \
  inline LibType rotate(LibType point, const LibType& angles) { \
    point = rotateX(point, angles.x);                           \
    point = rotateY(point, angles.y);                           \
    point = rotateZ(point, angles.z);                           \
    return point;                                               \
  }
RotateFunctions(vec3);
RotateFunctions(aiVector3D);
}  // namespace MathUtils