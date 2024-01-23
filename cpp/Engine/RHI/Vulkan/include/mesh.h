#pragma once

#include "Engine/Utility/include/TypeUtils.h"
#include "base.h"
#include "buffer.h"
#include "data.h"
#include "texture.h"
#include "uniform.h"

class Mesh : public Base {
  const MeshData* bridge;

  Data data;
  Buffer buffer;
  Descriptor descriptor;
  std::vector<Texture> textures;

  void ParseTextures(const Device& device, const Render& render);

  void ParseVertexAndIndex();

  void ParseBufferAndDescriptor(
      const Device& device, const Render& render,
      const VkDescriptorSetLayout& descriptorSetLayout);

 public:
  [[nodiscard]] const VkBuffer& GetIndexBuffer() const {
    return buffer.GetIndexBuffer();
  }

  [[nodiscard]] const VkBuffer& GetVertexBuffer() const {
    return buffer.GetVertexBuffer();
  }

  [[nodiscard]] const std::vector<uint32_t>& GetIndices() const {
    return data.GetIndices();
  }

  [[nodiscard]] const std::vector<Vertex>& GetVertices() const {
    return data.GetVertices();
  }

  [[nodiscard]] const DescriptorSets& GetDescriptorSets() const {
    return descriptor.GetDescriptorSets();
  }

  [[nodiscard]] const VkDescriptorSet& GetDescriptorSetByIndex(
      const uint32_t index) const {
    return descriptor.GetDescriptorSetByIndex(index);
  }

  void UpdateUniformBuffer(const uint32_t currentImage) const {
    descriptor.UpdateUniformBuffer(currentImage);
  }

  template <typename... Args>
  explicit Mesh(Base* owner, Args&&... args) : Base(owner) {
    CreateMesh(std::forward<Args>(args)...);
  }
  ~Mesh() override = default;
  virtual void TriggerRegisterMember() override {
    RegisterMember(data, buffer, descriptor);
  }

  void CreateMesh(const Device& device, const Render& render,
                  const MeshData* inData,
                  const VkDescriptorSetLayout& descriptorSetLayout);

  void DestroyMesh(const VkDevice& device, const Render& render) const;

  [[nodiscard]] const bool GetAlive() const { return bridge->state.alive; }
  const glm::mat4x4* GetModelMatrix();
  const glm::mat4x4* GetViewMatrix();
  const glm::mat4x4* GetProjMatrix();
  const glm::vec4* GetBaseColor();
  const float GetRoughness();
  const float GetMetallic();
};
