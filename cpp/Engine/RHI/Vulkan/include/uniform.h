#pragma once
#pragma once

#include <vulkan/vulkan_core.h>

#include "Engine/Utility/include/TypeUtils.h"
#include "base.h"
#include "buffer.h"

#define UniformBufferNum 4

#define DEFINE_GET_DESCRIPTOR_SET(lower, upper)                            \
  [[nodiscard]] const DescriptorSets& Get##upper##DescriptorSets() const { \
    return lower##DescriptorSets;                                          \
  }                                                                        \
  [[nodiscard]] const VkDescriptorSet& Get##upper##DescriptorSetByIndex(   \
      const uint32_t index) const {                                        \
    return lower##DescriptorSets[index];                                   \
  }

class Mesh;
class Device;
class Render;
class Texture;
class Descriptor;
using DescriptorSets = std::vector<VkDescriptorSet>;

class TransformBuffer : public UniformBuffer {
 public:
  std::weak_ptr<MeshData> GetBridgeData();
  void UpdateUniformBuffer(const uint32_t currentImage);
};

class Descriptor : public Base {
  BufferManager* bufferManager = nullptr;

  BaseCamera* cameraPointer = nullptr;
  CameraBuffer* cameraBuffer = nullptr;

  BaseMaterial* materialPointer = nullptr;
  MaterialBuffer* materialBuffer = nullptr;

  LightChannel* lightChannelPointer = nullptr;
  LightChannelBuffer* lightChannelBuffer = nullptr;

  TransformBuffer transformBuffer;

  VkDescriptorPool colorDescriptorPool;
  VkDescriptorPool zPrePassDescriptorPool;
  VkDescriptorPool shadowMapDescriptorPool;

  DescriptorSets colorDescriptorSets;
  DescriptorSets zPrePassDescriptorSets;
  DescriptorSets shadowMapDescriptorSets;

  void CreateColorDescriptorPool(const VkDevice& device, const Render& render,
                                 size_t textureNum);
  void CreateZPrePassDescriptorPool(const VkDevice& device,
                                    const Render& render);
  void CreateShadowMapDescriptorPool(const VkDevice& device,
                                     const Render& render);

  void CreateColorDescriptorSets(
      const VkDevice& device, const Render& render,
      const VkDescriptorSetLayout& colorDescriptorSetLayout,
      const std::vector<Texture>& textures);
  void CreateZPrePassDescriptorSets(
      const VkDevice& device, const Render& render,
      const VkDescriptorSetLayout& zPrePassDescriptorSetLayout);
  void CreateShadowMapDescriptorSets(
      const VkDevice& device, const Render& render,
      const VkDescriptorSetLayout& shadowMapDescriptorSetLayout);

  void UpdateBufferPointers();
  void CreateUniformBuffer(const Device& device, const Render& render);
  void DestroyUniformBuffer(const VkDevice& device, const Render& render);

 public:
  template <typename... Args>
  explicit Descriptor(Base* owner, Args&&... args) : Base(owner) {
    CreateDescriptor(std::forward(args)...);
  }
  Descriptor() = default;
  ~Descriptor() override = default;
  std::weak_ptr<MeshData> GetBridgeData();

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

  DEFINE_GET_DESCRIPTOR_SET(color, Color)
  DEFINE_GET_DESCRIPTOR_SET(zPrePass, ZPrePass)
  DEFINE_GET_DESCRIPTOR_SET(shadowMap, ShadowMap)

  void CreateDescriptor(
      const Device& device, const Render& render,
      const std::vector<Texture>& textures,
      const VkDescriptorSetLayout& colorDescriptorSetLayout,
      const VkDescriptorSetLayout& zPrePassDescriptorSetLayout,
      const VkDescriptorSetLayout& shadowMapDescriptorSetLayout);
  void DestroyDesciptor(const VkDevice& device, const Render& render);
};

#undef DEFINE_GET_DESCRIPTOR_SET