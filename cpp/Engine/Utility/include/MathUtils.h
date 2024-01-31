#pragma once

#include <assimp/types.h>

#include <cmath>
#include <glm/vec3.hpp>

namespace MathUtils {
using namespace glm;
using namespace std;

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
  inline LibType rotate(LibType point, const LibType angles) {  \
    point = rotateX(point, angles.x);                           \
    point = rotateY(point, angles.y);                           \
    point = rotateZ(point, angles.z);                           \
    return point;                                               \
  }
RotateFunctions(vec3);
RotateFunctions(aiVector3D);
}  // namespace MathUtils