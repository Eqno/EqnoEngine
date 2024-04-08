#include "../include/shader.h"

#include <Engine/Utility/include/FileUtils.h>
#include <glslc/file_includer.h>
#include <libshaderc_util/file_finder.h>

#include <filesystem>
#include <iostream>

namespace ShaderRcStatic {
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
}  // namespace ShaderRcStatic

Shader::Shader(const Definitions& definitions) { AddDefinitions(definitions); }
void Shader::AddDefinitions(const Definitions& definitions) {
  for (const auto& [name, value] : definitions) {
    ShaderRcStatic::options.AddMacroDefinition(name, value);
  }
}
void Shader::SetFileIncluder(const std::vector<std::string>& searchPaths) {
  ShaderRcStatic::fileFinder.search_path() = searchPaths;
  ShaderRcStatic::options.SetIncluder(
      std::make_unique<glslc::FileIncluder>(&ShaderRcStatic::fileFinder));
}
void Shader::SetOptimizationLevel(shaderc_optimization_level level) {
  ShaderRcStatic::options.SetOptimizationLevel(level);
}

const ShaderTypeInfo& Shader::GetTypeByName(const std::string& glslPath) {
  return ShaderRcStatic::shaderTypes
      .find(glslPath.substr(glslPath.find('.') + 1))
      ->second;
}

void Shader::CompileFromGLSLToSPV(const std::string& glslPath,
                                  const std::string& shaderPath) {
  const std::string glslCode =
      FileUtils::ReadFileAsString(shaderPath + "/" + glslPath);

  const auto module = ShaderRcStatic::compiler.CompileGlslToSpv(
      glslCode.c_str(), glslCode.size(), GetTypeByName(glslPath).kind,
      glslPath.c_str(), ShaderRcStatic::options);

  if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
    std::cerr << module.GetErrorMessage();
  }
  const std::vector spvCode(module.cbegin(), module.cend());
  FileUtils::WriteFileAsUIntegers(shaderPath + "/../spv/" + glslPath,
                                  std::ios::binary, spvCode);
}

UIntegers Shader::ReadSPVFileAsBinary(const std::string& spvPath,
                                      const std::string& shaderPath) {
  return FileUtils::ReadFileAsUIntegers(shaderPath + "/../spv/" + spvPath,
                                        std::ios::ate | std::ios::binary);
}

UIntegers Shader::ReadGLSLFileAsBinary(const std::string& glslPath,
                                       const std::string& shaderPath) {
  CompileFromGLSLToSPV(glslPath, shaderPath);
  return ReadSPVFileAsBinary(glslPath, shaderPath);
}

VkShaderModule Shader::CreateModule(const UIntegers& code,
                                    const VkDevice& device) {
  const VkShaderModuleCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = code.size(),
      .pCode = code.data(),
  };
  if (VkShaderModule shaderModule;
      vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) ==
      VK_SUCCESS) {
    return shaderModule;
  }
  throw std::runtime_error("failed to create shader module!");
}

ShaderStages Shader::AutoCreateStages(const VkDevice& device,
                                      const std::string& rootPath,
                                      const std::string& shaderPath) const {
  ShaderStages shaderStages;
  for (const auto& fileInfo :
       std::filesystem::directory_iterator(rootPath + shaderPath + "/glsl")) {
    const auto glslPath = fileInfo.path().filename().string();
    if (ShaderRcStatic::shaderTypes.find(glslPath.substr(
            glslPath.find('.') + 1)) != ShaderRcStatic::shaderTypes.end()) {
      shaderModules.emplace_back(CreateModule(
          ReadGLSLFileAsBinary(glslPath, rootPath + shaderPath + "/glsl"),
          device));
      shaderStages[GetTypeByName(glslPath).type].push_back({
          .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .stage = GetTypeByName(glslPath).stage,
          .module = shaderModules.back(),
          .pName = GetTypeByName(glslPath).entrance.c_str(),
      });
    }
  }
  return shaderStages;
}

[[nodiscard]] std::vector<ShaderStages> Shader::AutoCreateStagesSet(
    const VkDevice& device, const std::string& rootPath,
    const std::vector<std::string>& shaderPaths) const {
  std::vector<ShaderStages> shaderStagesSet;
  for (const std::string& shaderPath : shaderPaths) {
    shaderStagesSet.emplace_back(
        AutoCreateStages(device, rootPath, shaderPath));
  }
  return shaderStagesSet;
}

void Shader::DestroyModules(const VkDevice& device) const {
  for (const auto& shaderModule : shaderModules) {
    vkDestroyShaderModule(device, shaderModule, nullptr);
  }
  shaderModules.clear();
}
