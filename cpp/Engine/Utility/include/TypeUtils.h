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
class BaseCamera;
class BaseMaterial;
class LightChannel;
class BufferManager;

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

using MaterialInfo =
    std::pair<std::string, std::vector<std::pair<std::string, std::string>>>;

inline constexpr int MaxLightNum = 200;
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
  glm::vec3 pos;
  glm::vec4 color;
  glm::vec3 normal;
  glm::vec3 tangent;
  glm::vec3 texCoord;
};

enum class TextureType {
  Unset = 0,
  BaseColor = 1,
  Roughness = 2,
  Metallic = 3,
  Normal = 4,
  AO = 5,
};

inline std::unordered_map<std::string, TextureType> TextureTypeMap{
    {"BaseColor", TextureType::BaseColor},
    {"Roughness", TextureType::Roughness},
    {"Metallic", TextureType::Metallic},
    {"Normal", TextureType::Normal},
    {"AO", TextureType::AO},
};

struct TextureData {
  TextureType type;
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
  alignas(4) int id;
  alignas(4) LightType type;
  alignas(4) float intensity;
  alignas(16) glm::vec3 pos;
  alignas(16) glm::vec4 color;
  alignas(16) glm::vec3 normal;
  alignas(16) glm::mat4 viewMatrix;
  alignas(16) glm::mat4 projMatrix;
};

struct LightChannelData {
  alignas(4) unsigned int num = 0;
  alignas(16) LightData object[MaxLightNum];
};

struct UniformData {
  std::weak_ptr<BaseCamera> camera;
  std::weak_ptr<BaseMaterial> material;
  std::weak_ptr<LightChannel> lightChannel;

  glm::mat4* modelMatrix = nullptr;
  BufferManager* bufferManager = nullptr;
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