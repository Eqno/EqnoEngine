#pragma once

#include <vulkan/vulkan_core.h>

#include "base.h"
#include "mesh.h"
#include "pipeline.h"
#include "shader.h"

#define DEFINE_GET_PIPELINE_MEMBER(member)                                    \
  [[nodiscard]] const VkPipeline& Get##member##GraphicsPipeline() const {     \
    return pipeline.Get##member##GraphicsPipeline();                          \
  }                                                                           \
  [[nodiscard]] const VkPipelineLayout& Get##member##PipelineLayout() const { \
    return pipeline.Get##member##PipelineLayout();                            \
  }                                                                           \
  [[nodiscard]] const VkDescriptorSetLayout&                                  \
      Get##member##DescriptorSetLayout() const {                              \
    return pipeline.Get##member##DescriptorSetLayout();                       \
  }

class Render;

class Draw : public Base {
  Shader shader;
  Pipeline pipeline;
  std::list<Mesh*> meshes;

 public:
  [[nodiscard]] int GetShaderFallbackIndex() {
    return pipeline.GetShaderFallbackIndex();
  }

  DEFINE_GET_PIPELINE_MEMBER(Color)
  DEFINE_GET_PIPELINE_MEMBER(ZPrePass)
  DEFINE_GET_PIPELINE_MEMBER(ShadowMap)

  std::list<Mesh*>& GetMeshes() { return meshes; }

  explicit Draw(Base* owner) : Base(owner) {}
  ~Draw() override = default;

  virtual void TriggerRegisterMember() override {
    RegisterMember(shader, pipeline);
  }
  template <typename... Args>
  void TriggerInitComponent(Args&&... args) {
    CreateDrawResource(std::forward<Args>(args)...);
  }

  void CreateDrawResource(const Device& device, const std::string& rootPath,
                          const std::vector<std::string>& shaderPaths,
                          const std::string& zPrePassShaderPath,
                          const std::string& shadowMapShaderPath,
                          const VkRenderPass& colorRenderPass,
                          const VkRenderPass& zPrePassRenderPass,
                          const VkRenderPass& shadowMapRenderPass,
                          const int texCount);

  void LoadDrawResource(const Device& device, const Render& render,
                        std::weak_ptr<MeshData> data);

  void DestroyDrawResource(const VkDevice& device, const Render& render);
};

#undef DEFINE_GET_PIPELINE_MEMBER