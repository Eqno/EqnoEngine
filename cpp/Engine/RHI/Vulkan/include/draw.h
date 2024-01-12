#pragma once

#include "base.h"
#include "mesh.h"
#include "pipeline.h"
#include "shader.h"

class Render;

class Draw : public Base {
  Shader shader;
  Pipeline pipeline;
  std::vector<Mesh*> meshes;

 public:
  [[nodiscard]] const VkPipeline& GetGraphicsPipeline() const {
    return pipeline.GetGraphicsPipeline();
  }

  [[nodiscard]] const VkPipelineLayout& GetPipelineLayout() const {
    return pipeline.GetPipelineLayout();
  }

  [[nodiscard]] const VkDescriptorSetLayout& GetDescriptorSetLayout() const {
    return pipeline.GetDescriptorSetLayout();
  }

  const std::vector<Mesh*>& GetMeshes() const { return meshes; }

  template <typename... Args>
  explicit Draw(Base* owner, Args&&... args) : Base(owner) {
    CreateDrawResource(std::forward<Args>(args)...);
  }
  ~Draw() override = default;
  virtual void TriggerRegisterMember() override {
    RegisterMember(shader, pipeline);
  }

  void CreateDrawResource(const Device& device, const std::string& shaderPath,
                          const VkRenderPass& renderPass);

  void LoadDrawResource(const Device& device, const Render& render,
                        const MeshData* data);

  void DestroyDrawResource(const VkDevice& device, const Render& render) const;
};
