#include "../include/shader.h"

#include <Engine/Utility/include/FileUtils.h>

#include <filesystem>
#include <iostream>

Shader::Shader(const Definitions& definitions) { AddDefinitions(definitions); }
void Shader::AddDefinitions(const Definitions& definitions) {
  for (const auto& [name, value] : definitions) {
    options.AddMacroDefinition(name, value);
  }
}
void Shader::SetFileIncluder(const std::vector<std::string>& searchPaths) {
  fileFinder.search_path() = searchPaths;
  options.SetIncluder(std::make_unique<glslc::FileIncluder>(&fileFinder));
}
void Shader::SetOptimizationLevel(shaderc_optimization_level level) {
  options.SetOptimizationLevel(level);
}
void Shader::SetGenerateDebugInfo(const bool flag) {
  if (flag == true) {
    options.SetGenerateDebugInfo();
  }
}

const ShaderTypeInfo& Shader::GetTypeByName(const std::string& glslPath) {
  return shaderTypes.find(glslPath.substr(glslPath.find('.') + 1))->second;
}

void Shader::CompileFromGLSLToSPV(const std::string& glslPath,
                                  const std::string& shaderPath) {
  const std::string glslCode =
      FileUtils::ReadFileAsString(shaderPath + "/" + glslPath);

  const auto module = compiler.CompileGlslToSpv(
      glslCode.c_str(), glslCode.size(), GetTypeByName(glslPath).kind,
      glslPath.c_str(), options);

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
  PRINT_AND_THROW_ERROR("failed to create shader module!");
}

ShaderStages Shader::AutoCreateStages(const VkDevice& device,
                                      const std::string& rootPath,
                                      const std::string& shaderPath) {
  ShaderStages shaderStages;
  for (const auto& fileInfo :
       std::filesystem::directory_iterator(rootPath + shaderPath + "/glsl")) {
    const auto glslPath = fileInfo.path().filename().string();
    if (shaderTypes.find(glslPath.substr(glslPath.find('.') + 1)) !=
        shaderTypes.end()) {
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
    const std::vector<std::string>& shaderPaths) {
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
