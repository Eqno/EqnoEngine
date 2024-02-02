#pragma once

#include <vulkan/vulkan_core.h>

#include <shaderc/shaderc.hpp>
#include <unordered_map>

#include "Engine/Utility/include/TypeUtils.h"
#include "base.h"

using ShaderStages = std::vector<VkPipelineShaderStageCreateInfo>;
using Definitions = std::vector<std::pair<std::string, std::string>>;

struct ShaderTypeInfo {
  shaderc_shader_kind kind;
  VkShaderStageFlagBits stage;
  std::string entrance;
};

class Shader : public Base {
  mutable std::vector<VkShaderModule> shaderModules;
  static const ShaderTypeInfo& GetTypeByName(const std::string& glslPath);

  [[nodiscard]] static UIntegers ReadSPVFileAsBinary(
      const std::string& spvPath, const std::string& shaderPath);

  [[nodiscard]] static UIntegers ReadGLSLFileAsBinary(
      const std::string& glslPath, const std::string& shaderPath);

  static void CompileFromGLSLToSPV(const std::string& glslPath,
                                   const std::string& shaderPath);

  static VkShaderModule CreateModule(const UIntegers& code,
                                     const VkDevice& device);

 public:
  explicit Shader() {}
  explicit Shader(const Definitions& definitions);

  void AddDefinitions(const Definitions& definitions);
  void SetFileIncluder();
  void SetOptimizationLevel(shaderc_optimization_level level);
  void DestroyModules(const VkDevice& device) const;

  [[nodiscard]] ShaderStages AutoCreateStages(
      const VkDevice& device, const std::string& rootPath,
      const std::string& shaderPaths) const;

  [[nodiscard]] std::vector<ShaderStages> AutoCreateStagesSet(
      const VkDevice& device, const std::string& rootPath,
      const std::vector<std::string>& shaderPath) const;
};
