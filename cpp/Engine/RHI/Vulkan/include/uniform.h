#pragma once

#include <vulkan/vulkan_core.h>

#include "Engine/Utility/include/TypeUtils.h"
#include "base.h"

#define UniformBufferNum 4

class Mesh;
class Device;
class Render;
class Texture;
class Descriptor;

using UniformMapped = std::vector<void*>;
using UniformBuffers = std::vector<VkBuffer>;
using UniformMemories = std::vector<VkDeviceMemory>;
using DescriptorSets = std::vector<VkDescriptorSet>;

class UniformBuffer : public Base {
  const MeshData* bridge = nullptr;

  UniformBuffers uniformBuffers;
  UniformMemories uniformBuffersMemory;
  UniformMapped uniformBuffersMapped;

 public:
  const MeshData* GetBridgeData();
  [[nodiscard]] const UniformBuffers& GetUniformBuffers() const {
    return uniformBuffers;
  }

  [[nodiscard]] const VkBuffer& GetUniformBufferByIndex(
      const size_t index) const {
    return uniformBuffers[index];
  }

  void CreateUniformBuffers(const Device& device, const Render& render);
  void UpdateUniformBuffer(const uint32_t currentImage);
  void DestroyUniformBuffer(const VkDevice& device, const Render& render) const;
};

class Descriptor : public Base {
  const MeshData* bridge = nullptr;

  UniformBuffer uniformBuffer;
  VkDescriptorPool descriptorPool;
  DescriptorSets descriptorSets;

  void CreateDescriptorSets(const VkDevice& device, const Render& render,
                            const VkDescriptorSetLayout& descriptorSetLayout,
                            const std::vector<Texture>& textures);

  void CreateDescriptorPool(const VkDevice& device, const Render& render,
                            size_t textureNum);

  void CreateUniformBuffer(const Device& device, const Render& render);
  void DestroyUniformBuffers(const VkDevice& device,
                             const Render& render) const;

 public:
  template <typename... Args>
  explicit Descriptor(Base* owner, Args&&... args) : Base(owner) {
    CreateDescriptor(std::forward(args)...);
  }
  Descriptor() = default;
  ~Descriptor() override = default;

  const MeshData* GetBridgeData();
  virtual void TriggerRegisterMember() override {
    this;
    RegisterMember(uniformBuffer);
  }

  void UpdateUniformBuffer(const uint32_t currentImage) {
    uniformBuffer.UpdateUniformBuffer(currentImage);
  }

  [[nodiscard]] const UniformBuffer& GetUniformBuffer() const {
    return uniformBuffer;
  }

  [[nodiscard]] const UniformBuffers& GetUniformBuffers() const {
    return uniformBuffer.GetUniformBuffers();
  }

  [[nodiscard]] const VkBuffer& GetUniformBufferByIndex(
      const size_t index) const {
    return uniformBuffer.GetUniformBufferByIndex(index);
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
  void DestroyDesciptor(const VkDevice& device, const Render& render) const;
};
