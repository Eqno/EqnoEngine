#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <assimp/color4.h>
#include <assimp/vector3.h>
#include <stb_image.h>

#include <chrono>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class Vertex;
class BaseLight;

using Vertexes = std::vector<Vertex>;
using Integers = std::vector<int32_t>;
using UIntegers = std::vector<uint32_t>;

using Strings = std::vector<std::string>;
using CStrings = std::vector<const char*>;
using StringSet = std::set<std::string>;

using seconds = std::chrono::duration<float>;
using milliseconds = std::chrono::duration<float, std::milli>;
using microseconds = std::chrono::duration<float, std::micro>;
using nanoseconds = std::chrono::duration<float, std::nano>;

inline std::string StringUnset = "Unset";
inline constexpr int MaxLightNum = 500;

struct VertexData {
  aiVector3D pos;
  aiColor4D color;
  aiVector3D normal;
  aiVector3D tangent;
  aiVector3D texCoord;
};

struct TextureData {
  int width;
  int height;
  int channels;
  stbi_uc* data;
};

enum class LightType {
  Unset = 0,
  Direct = 1,
  Point = 2,
  Spot = 3,
};

struct CameraData {
  alignas(16) glm::vec3 pos;
  alignas(16) glm::vec3 normal;
};

struct MaterialData {
  alignas(16) glm::vec4 color;
  alignas(4) float roughness;
  alignas(4) float metallic;
};

struct TransformData {
  alignas(16) glm::mat4 modelMatrix;
  alignas(16) glm::mat4 viewMatrix;
  alignas(16) glm::mat4 projMatrix;
};

struct LightData {
  alignas(4) LightType type;
  alignas(4) float intensity;
  alignas(16) glm::vec3 pos;
  alignas(16) glm::vec4 color;
  alignas(16) glm::vec3 normal;
};

struct LightsData {
  alignas(4) unsigned int num;
  alignas(16) LightData object[MaxLightNum];
};

struct UniformData {
  std::string* shader;
  MaterialData* material;
  std::vector<BaseLight*>* lights;

  glm::vec3* cameraPosition;
  glm::vec3* cameraForward;

  glm::mat4* modelMatrix;
  glm::mat4* viewMatrix;
  glm::mat4* projMatrix;
};

struct StateData {
  bool alive;
};

struct MeshData {
  StateData state;
  std::string name;
  UniformData uniform;
  std::vector<uint32_t> indices;
  std::vector<VertexData> vertices;
  std::vector<TextureData> textures;
};

template <typename... Args>
class Reflection {
  static std::unordered_map<std::string, std::function<void*(Args&&...)>>
      classMap;

  static void* classFromName(std::string name, Args&&... args) {
    if (classMap.find(name) == classMap.end()) return nullptr;
    return classMap[name](std::forward<Args>(args)...);
  }

 public:
  Reflection(std::string name, std::function<void*(Args&&...)> func) {
    classMap.insert(std::make_pair(name, func));
  }
};

inline glm::mat4x4 Mat4x4Zero = glm::mat4x4(0);
inline glm::vec3 Vec3Zero = glm::vec3(0);