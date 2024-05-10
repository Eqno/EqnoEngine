#pragma once

#include <glslc/file_includer.h>
#include <libshaderc_util/file_finder.h>
#include <vulkan/vulkan_core.h>

#include <shaderc/shaderc.hpp>
#include <unordered_map>

#include "Engine/Utility/include/TypeUtils.h"
#include "base.h"

using ShaderStages =
    std::unordered_map<PipelineType,
                       std::vector<VkPipelineShaderStageCreateInfo>>;
using Definitions = std::vector<std::pair<std::string, std::string>>;

struct ShaderTypeInfo {
  PipelineType type;
  shaderc_shader_kind kind;
  VkShaderStageFlagBits stage;
  std::string entrance;
};

class Shader : public Base {
  shaderc::Compiler compiler;
  shaderc::CompileOptions options;
  shaderc_util::FileFinder fileFinder;
  std::unordered_map<std::string, ShaderTypeInfo> shaderTypes{
      // Forward shading
      {"vert",
       {PipelineType::Forward, shaderc_glsl_vertex_shader,
        VK_SHADER_STAGE_VERTEX_BIT, "main"}},
      {"frag",
       {PipelineType::Forward, shaderc_glsl_fragment_shader,
        VK_SHADER_STAGE_FRAGMENT_BIT, "main"}},

      // Deferred shading
      {"verto",
       {PipelineType::DeferredOutputGBuffer, shaderc_glsl_vertex_shader,
        VK_SHADER_STAGE_VERTEX_BIT, "main"}},
      {"frago",
       {PipelineType::DeferredOutputGBuffer, shaderc_glsl_fragment_shader,
        VK_SHADER_STAGE_FRAGMENT_BIT, "main"}},

      {"vertp",
       {PipelineType::DeferredProcessGBuffer, shaderc_glsl_vertex_shader,
        VK_SHADER_STAGE_VERTEX_BIT, "main"}},
      {"fragp",
       {PipelineType::DeferredProcessGBuffer, shaderc_glsl_fragment_shader,
        VK_SHADER_STAGE_FRAGMENT_BIT, "main"}},
  };

  std::string shaderPath = "Unset";
  mutable std::vector<VkShaderModule> shaderModules;
  const ShaderTypeInfo& GetTypeByName(const std::string& glslPath);

  UIntegers ReadSPVFileAsBinary(const std::string& spvPath,
                                const std::string& shaderPath);

  UIntegers ReadGLSLFileAsBinary(const std::string& glslPath,
                                 const std::string& shaderPath);

  void CompileFromGLSLToSPV(const std::string& glslPath,
                            const std::string& shaderPath);

  static VkShaderModule CreateModule(const UIntegers& code,
                                     const VkDevice& device);

 public:
  explicit Shader() {}
  explicit Shader(const Definitions& definitions);

  void AddDefinitions(const Definitions& definitions);
  void SetFileIncluder(const std::vector<std::string>& searchPaths);
  void SetOptimizationLevel(shaderc_optimization_level level);
  void SetGenerateDebugInfo(const bool flag);
  void DestroyModules(const VkDevice& device) const;

  [[nodiscard]] ShaderStages AutoCreateStages(const VkDevice& device,
                                              const std::string& rootPath,
                                              const std::string& shaderPaths);

  [[nodiscard]] std::vector<ShaderStages> AutoCreateStagesSet(
      const VkDevice& device, const std::string& rootPath,
      const std::vector<std::string>& shaderPath);

  const std::string& GetShaderPath() { return shaderPath; }
  void SetShaderPath(const std::string& shaderPath) {
    this->shaderPath = shaderPath;
  }
};
