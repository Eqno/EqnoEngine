#pragma once

#include <vulkan/vulkan_core.h>

#include <string>

#include "base.h"

class Shader;
class Device;

class Pipeline : public Base {
  VkDescriptorSetLayout descriptorSetLayout{};
  VkPipelineLayout pipelineLayout{};
  VkPipeline graphicsPipeline{};

  void CreateGraphicsPipeline(const VkDevice& device, const Shader& shader,
                              const std::string& shaderPath,
                              const VkRenderPass& renderPass);
  void CreateDescriptorSetLayout(const VkDevice& device);

 public:
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
                      const std::string& shaderPath,
                      const VkRenderPass& renderPass);
  void DestroyPipeline(const VkDevice& device) const;
};
