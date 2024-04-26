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
#include <utility>
#include <vector>

#define PRINT_AND_THROW_ERROR(errorMsg) \
  std::cout << (errorMsg) << std::endl; \
  throw std::runtime_error((errorMsg))

#define PRINT_ERROR(errorMsg) std::cout << (errorMsg) << std::endl

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

inline constexpr int MaxLightNum = 2;
inline constexpr int MaxPipelineNum = 10;

inline constexpr shaderc_optimization_level ShaderOptimizationLevel =
    shaderc_optimization_level_zero;
inline const std::vector<std::string> ShaderSearchPaths = {
    "Assets/Shaders/"
    /*, "Assets/Shaders/GLSLLibrary/"*/
};

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

enum class PipelineType {
  Unset = 0,
  Forward = 1,
  DeferredOutputGBuffer = 2,
  DeferredProcessGBuffer = 3,
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

struct PipelineData {
  alignas(4) int id;
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
  glm::mat4x4* modelMatrix;
  std::weak_ptr<BaseCamera> camera;
  std::weak_ptr<BaseMaterial> material;
  std::weak_ptr<LightChannel> lightChannel;
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

namespace FuncUtils {
// Reflection
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
// Params Unpack
template <typename ReturnType, typename... Args>
struct function_traits_defs {
  static constexpr size_t arity = sizeof...(Args);

  using result_type = ReturnType;
  template <size_t i>
  struct arg {
    using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
  };
};

template <typename T>
struct function_traits_impl;

template <typename ReturnType, typename... Args>
struct function_traits_impl<ReturnType(Args...)>
    : function_traits_defs<ReturnType, Args...> {};

template <typename ReturnType, typename... Args>
struct function_traits_impl<ReturnType (*)(Args...)>
    : function_traits_defs<ReturnType, Args...> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits_impl<ReturnType (ClassType::*)(Args...)>
    : function_traits_defs<ReturnType, Args...> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits_impl<ReturnType (ClassType::*)(Args...) const>
    : function_traits_defs<ReturnType, Args...> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits_impl<ReturnType (ClassType::*)(Args...) const&>
    : function_traits_defs<ReturnType, Args...> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits_impl<ReturnType (ClassType::*)(Args...) const&&>
    : function_traits_defs<ReturnType, Args...> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits_impl<ReturnType (ClassType::*)(Args...) volatile>
    : function_traits_defs<ReturnType, Args...> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits_impl<ReturnType (ClassType::*)(Args...) volatile&>
    : function_traits_defs<ReturnType, Args...> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits_impl<ReturnType (ClassType::*)(Args...) volatile&&>
    : function_traits_defs<ReturnType, Args...> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits_impl<ReturnType (ClassType::*)(Args...) const volatile>
    : function_traits_defs<ReturnType, Args...> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits_impl<ReturnType (ClassType::*)(Args...) const volatile&>
    : function_traits_defs<ReturnType, Args...> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits_impl<ReturnType (ClassType::*)(Args...) const volatile&&>
    : function_traits_defs<ReturnType, Args...> {};

template <typename T, typename V = void>
struct function_traits : function_traits_impl<T> {};

template <typename T>
struct function_traits<T, decltype((void)&T::operator())>
    : function_traits_impl<decltype(&T::operator())> {};

template <size_t... Indices>
struct indices {
  using next = indices<Indices..., sizeof...(Indices)>;
};
template <size_t N>
struct build_indices {
  using type = typename build_indices<N - 1>::type::next;
};
template <>
struct build_indices<0> {
  using type = indices<>;
};
template <size_t N>
using BuildIndices = typename build_indices<N>::type;

namespace Details {
template <typename FuncType, typename VecType, size_t... I,
          typename Traits = function_traits<FuncType>,
          typename ReturnT = typename Traits::result_type>
ReturnT Call(FuncType& func, const VecType& args, indices<I...>) {
  return func(args[I]...);
}
}  // namespace Details
template <typename FuncType, typename VecType,
          typename Traits = function_traits<FuncType>>
auto UnpackCall(FuncType& func, const VecType& args) {
  return Details::Call(func, args, BuildIndices<Traits::arity>());
}
}  // namespace FuncUtils
