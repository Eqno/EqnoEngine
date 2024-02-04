#pragma once

#include <vulkan/vulkan_core.h>

#include "Engine/Utility/include/TypeUtils.h"
#include "base.h"
#include "buffer.h"

#define UniformBufferNum 4

class Mesh;
class Device;
class Render;
class Texture;
class Descriptor;
using DescriptorSets = std::vector<VkDescriptorSet>;

class TransformBuffer : public UniformBuffer {
  const MeshData* bridge = nullptr;

 public:
  const MeshData* GetBridgeData();
  void UpdateUniformBuffer(const uint32_t currentImage);
};

class Descriptor : public Base {
  const MeshData* bridge = nullptr;

  CameraBuffer* cameraBuffer;
  MaterialBuffer* materialBuffer;
  LightChannelBuffer* lightChannelBuffer;

  TransformBuffer transformBuffer;
  VkDescriptorPool descriptorPool;
  DescriptorSets descriptorSets;

  void CreateDescriptorSets(const VkDevice& device, const Render& render,
                            const VkDescriptorSetLayout& descriptorSetLayout,
                            const std::vector<Texture>& textures);

  void CreateDescriptorPool(const VkDevice& device, const Render& render,
                            size_t textureNum);

  void CreateUniformBuffer(const Device& device, const Render& render);
  void DestroyUniformBuffer(const VkDevice& device, const Render& render);

 public:
  template <typename... Args>
  explicit Descriptor(Base* owner, Args&&... args) : Base(owner) {
    CreateDescriptor(std::forward(args)...);
  }
  Descriptor() = default;
  ~Descriptor() override = default;
  const MeshData* GetBridgeData();

  virtual void TriggerRegisterMember() override {
    RegisterMember(transformBuffer);
  }
  void UpdateUniformBuffer(const uint32_t currentImage);

  [[nodiscard]] const TransformBuffer& GetUniformBuffer() const {
    return transformBuffer;
  }
  [[nodiscard]] const UniformBuffers& GetUniformBuffers() const {
    return transformBuffer.GetUniformBuffers();
  }
  [[nodiscard]] const VkBuffer& GetUniformBufferByIndex(
      const size_t index) const {
    return transformBuffer.GetUniformBufferByIndex(index);
  }
  [[nodiscard]] const DescriptorSets& GetDescriptorSets() const {
    return descriptorSets;
  }
  [[nodiscard]] const VkDescriptorSet& GetDescriptorSetByIndex(
      const uint32_t index) const {
    return descriptorSets[index];
  }

  void CreateDescriptor(const Device& device, const Render& render,
                        const VkDescriptorSetLayout& descriptorSetLayout,
                        const std::vector<Texture>& textures);
  void DestroyDesciptor(const VkDevice& device, const Render& render);
};
