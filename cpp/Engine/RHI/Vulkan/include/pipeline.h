#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

#include "base.h"

class Shader;
class Device;

class Pipeline : public Base {
  int shaderFallbackIndex = -1;

  VkDescriptorSetLayout descriptorSetLayout{};
  VkPipelineLayout pipelineLayout{};
  VkPipeline graphicsPipeline{};

  void CreateGraphicsPipeline(const VkDevice& device, const Shader& shader,
                              const std::string& rootPath,
                              const std::vector<std::string>& shaderPaths,
                              const VkRenderPass& renderPass);
  void CreateDescriptorSetLayout(const VkDevice& device, int texCount);

 public:
  [[nodiscard]] int GetShaderFallbackIndex() { return shaderFallbackIndex; }

  [[nodiscard]] const VkPipeline& GetGraphicsPipeline() const {
    return graphicsPipeline;
  }

  [[nodiscard]] const VkPipelineLayout& GetPipelineLayout() const {
    return pipelineLayout;
  }

  [[nodiscard]] const VkDescriptorSetLayout& GetDescriptorSetLayout() const {
    return descriptorSetLayout;
  }

  void CreatePipeline(const Device& device, const Shader& shader,
                      const std::string& rootPath,
                      const std::vector<std::string>& shaderPaths,
                      const VkRenderPass& renderPass, int texCount);
  void DestroyPipeline(const VkDevice& device) const;
};
