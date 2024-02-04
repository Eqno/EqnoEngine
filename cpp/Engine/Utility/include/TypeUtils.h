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
#include <shaderc/shaderc.hpp>
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

using MaterialInfo = std::pair<std::string, Strings>;

inline constexpr int MaxLightNum = 500;
inline constexpr shaderc_optimization_level ShaderOptimizationLevel =
    shaderc_optimization_level_zero;
inline const std::vector<std::string> ShaderSearchPaths = {
    "Assets/Shaders/GLSLLibrary/"};

inline std::string StringUnset = "Unset";

inline glm::mat4x4 Mat4x4Zero = glm::mat4x4(0);
inline glm::mat4x4 Mat4x4One = glm::mat4x4(1);

inline glm::vec3 Vec3Zero = glm::vec3(0);
inline glm::vec3 Vec3One = glm::vec3(1);

inline glm::vec4 Vec4Zero = glm::vec4(0);
inline glm::vec4 Vec4One = glm::vec4(1);

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
  Sun = 1,
  Point = 2,
  Spot = 3,
};

struct CameraData {
  alignas(16) glm::vec3 pos = Vec3Zero;
  alignas(16) glm::vec3 normal = Vec3Zero;
};

struct MaterialData {
  alignas(16) glm::vec4 color = Vec4One;
  alignas(4) float roughness = 1;
  alignas(4) float metallic = 0;
};

struct TransformData {
  alignas(16) glm::mat4 modelMatrix = Mat4x4One;
  alignas(16) glm::mat4 viewMatrix = Mat4x4One;
  alignas(16) glm::mat4 projMatrix = Mat4x4One;
};

struct LightData {
  alignas(4) LightType type = LightType::Unset;
  alignas(4) float intensity = 1;
  alignas(16) glm::vec3 pos = Vec3Zero;
  alignas(16) glm::vec4 color = Vec4One;
  alignas(16) glm::vec3 normal = Vec3Zero;
};

struct LightsData {
  alignas(4) unsigned int num = 0;
  alignas(16) LightData object[MaxLightNum];
};

struct UniformData {
  MaterialData* material;
  std::vector<std::string>* shaders;
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