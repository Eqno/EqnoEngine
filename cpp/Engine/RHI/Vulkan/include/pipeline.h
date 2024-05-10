#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

#include "base.h"

#define DEFINE_GET_PIPELINE_AND_DSL(lower, upper)                              \
  [[nodiscard]] const VkPipeline& Get##upper##GraphicsPipeline() const {       \
    return lower##GraphicsPipeline;                                            \
  }                                                                            \
  [[nodiscard]] const VkPipelineLayout& Get##upper##PipelineLayout() const {   \
    return lower##PipelineLayout;                                              \
  }                                                                            \
  [[nodiscard]] const VkDescriptorSetLayout& Get##upper##DescriptorSetLayout() \
      const {                                                                  \
    return lower##DescriptorSetLayout;                                         \
  }

class Shader;
class Device;
class Render;

class Pipeline : public Base {
  int pipelineId = -1;
  int shaderFallbackIndex = -1;

  VkDescriptorSetLayout colorDescriptorSetLayout;
  VkPipelineLayout colorPipelineLayout;
  VkPipeline colorGraphicsPipeline;

  VkDescriptorSetLayout deferredDescriptorSetLayout;
  VkPipelineLayout deferredPipelineLayout;
  VkPipeline deferredGraphicsPipeline;

  VkDescriptorSetLayout zPrePassDescriptorSetLayout;
  VkPipelineLayout zPrePassPipelineLayout;
  VkPipeline zPrePassGraphicsPipeline;

  VkDescriptorSetLayout shadowMapDescriptorSetLayout;
  VkPipelineLayout shadowMapPipelineLayout;
  VkPipeline shadowMapGraphicsPipeline;

  void CreatePipelineLayout(const VkDevice& device,
                            const VkDescriptorSetLayout& dstLayout,
                            VkPipelineLayout& pipelineLayout);
  void CreateColorGraphicsPipeline(const Device& device, Render& render,
                                   Shader& shader, const std::string& rootPath,
                                   const std::vector<std::string>& shaderPaths,
                                   const VkRenderPass& renderPass);
  void CreateZPrePassGraphicsPipeline(const Device& device, Shader& shader,
                                      const std::string& rootPath,
                                      const std::string& depthShaderPath,
                                      const VkRenderPass& renderPass);
  void CreateShadowMapGraphicsPipeline(const VkDevice& device, Render& render,
                                       Shader& shader,
                                       const std::string& rootPath,
                                       const std::string& depthShaderPath);

  void CreateColorDescriptorSetLayout(const VkDevice& device, Render& render,
                                      int texCount);
  void CreateZPrePassDescriptorSetLayout(const VkDevice& device);
  void CreateShadowMapDescriptorSetLayout(const VkDevice& device);

 public:
  [[nodiscard]] int GetShaderFallbackIndex() { return shaderFallbackIndex; }

  DEFINE_GET_PIPELINE_AND_DSL(color, Color)
  DEFINE_GET_PIPELINE_AND_DSL(deferred, Deferred)
  DEFINE_GET_PIPELINE_AND_DSL(zPrePass, ZPrePass)
  DEFINE_GET_PIPELINE_AND_DSL(shadowMap, ShadowMap)

  void CreatePipeline(const Device& device, Render& render, Shader& shader,
                      int texCount, const std::string& rootPath,
                      const std::vector<std::string>& shaderPaths,
                      const std::string& zPrePassShaderPath,
                      const std::string& shadowMapShaderPath);
  void DestroyPipeline(const VkDevice& device, const Render& render) const;

  int GetPipelineId() { return pipelineId; }
  void SetPipelineId(const int pipelineId) { this->pipelineId = pipelineId; }
};

#undef DEFINE_GET_PIPELINE_AND_DSL