#pragma once

#include "Engine/Utility/include/TypeUtils.h"
#include "base.h"
#include "buffer.h"
#include "data.h"
#include "texture.h"
#include "uniform.h"

#define DEFINE_GET_DESCRIPTOR_SET(member)                                   \
  [[nodiscard]] const DescriptorSets& Get##member##DescriptorSets() const { \
    return descriptor.Get##member##DescriptorSets();                        \
  }                                                                         \
  [[nodiscard]] const VkDescriptorSet& Get##member##DescriptorSetByIndex(   \
      const uint32_t index) const {                                         \
    return descriptor.Get##member##DescriptorSetByIndex(index);             \
  }

class Mesh : public Base {
  std::weak_ptr<MeshData> bridge;

  Data data;
  DataBuffer buffer;
  Descriptor descriptor;
  std::vector<Texture> textures;

  void ParseTextures(const Device& device, const Render& render);
  void ParseVertexAndIndex();
  void ParseBufferAndDescriptor(
      const Device& device, Render& render,
      const VkDescriptorSetLayout& colorDescriptorSetLayout,
      const VkDescriptorSetLayout& zPrePassDescriptorSetLayout,
      const VkDescriptorSetLayout& shadowMapDescriptorSetLayout);

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

  DEFINE_GET_DESCRIPTOR_SET(Color)
  DEFINE_GET_DESCRIPTOR_SET(ZPrePass)

  [[nodiscard]] const DescriptorSets& GetShadowMapDescriptorSetsByIndex(
      const uint32_t index) {
    return descriptor.GetShadowMapDescriptorSetsByIndex(index);
  }
  [[nodiscard]] const VkDescriptorSet& GetShadowMapDescriptorSetByIndices(
      const uint32_t lightId, const uint32_t currentFrame) {
    return descriptor.GetShadowMapDescriptorSetByIndices(lightId, currentFrame);
  }

  void UpdateColorDescriptorSets(const VkDevice& device, Render& render) {
    descriptor.UpdateColorDescriptorSets(device, render);
  }
  void UpdateUniformBuffer(const uint32_t currentImage) {
    descriptor.UpdateUniformBuffer(currentImage);
  }
  void UpdateShadowMapUniformBuffers(
      const Device& device, Render& render, const uint32_t currentImage,
      std::unordered_map<int, std::weak_ptr<BaseLight>>& lightsById) {
    descriptor.UpdateShadowMapUniformBuffers(device, render, currentImage,
                                             lightsById);
  }

  explicit Mesh(Base* owner) : Base(owner) {}
  ~Mesh() override = default;

  virtual void TriggerRegisterMember() override {
    RegisterMember(data, buffer, descriptor);
  }
  template <typename... Args>
  void TriggerInitComponent(Args&&... args) {
    CreateMesh(std::forward<Args>(args)...);
  }

  void CreateMesh(const Device& device, Render& render,
                  std::weak_ptr<MeshData> inData,
                  const VkDescriptorSetLayout& colorDescriptorSetLayout,
                  const VkDescriptorSetLayout& zPrePassDescriptorSetLayout,
                  const VkDescriptorSetLayout& shadowMapDescriptorSetLayout);

  void DestroyMesh(const VkDevice& device, const Render& render);

  [[nodiscard]] const bool GetAlive() const {
    if (auto bridgePtr = bridge.lock()) {
      return bridgePtr->state.alive;
    } else {
      return false;
    }
  }
  std::weak_ptr<MeshData> GetBridgeData() { return bridge; }
};

#undef DEFINE_GET_DESCRIPTOR_SET