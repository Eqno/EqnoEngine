#pragma once

#include "base.h"
#include "mesh.h"
#include "pipeline.h"
#include "shader.h"

class Render;

class Draw : public Base {
  Shader shader;
  Pipeline pipeline;
  std::list<Mesh*> meshes;

 public:
  [[nodiscard]] int GetShaderFallbackIndex() {
    return pipeline.GetShaderFallbackIndex();
  }

  [[nodiscard]] const VkPipeline& GetGraphicsPipeline() const {
    return pipeline.GetGraphicsPipeline();
  }

  [[nodiscard]] const VkPipelineLayout& GetPipelineLayout() const {
    return pipeline.GetPipelineLayout();
  }

  [[nodiscard]] const VkDescriptorSetLayout& GetDescriptorSetLayout() const {
    return pipeline.GetDescriptorSetLayout();
  }

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
                          const VkRenderPass& renderPass, const int texCount);

  void LoadDrawResource(const Device& device, const Render& render,
                        std::weak_ptr<MeshData> data);

  void DestroyDrawResource(const VkDevice& device, const Render& render);
};
