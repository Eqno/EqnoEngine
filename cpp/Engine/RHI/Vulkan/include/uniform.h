#pragma once
#pragma once

#include <vulkan/vulkan_core.h>

#include <mutex>

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
class BaseLight;
class Descriptor;
using DescriptorSets = std::vector<VkDescriptorSet>;

class TransformBuffer : public UniformBuffer {
 public:
  std::weak_ptr<MeshData> GetBridgeData();
  void UpdateUniformBuffer(const uint32_t currentImage);
};
class ShadowMapBuffer : public UniformBuffer {
 public:
  std::weak_ptr<MeshData> GetBridgeData();
  void UpdateUniformBuffer(const uint32_t currentImage,
                           BaseLight* shadowMapLight);
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
  std::unordered_map<uint32_t, ShadowMapBuffer> shadowMapBuffers;

  VkDescriptorPool colorDescriptorPool;
  VkDescriptorPool zPrePassDescriptorPool;
  std::unordered_map<uint32_t, VkDescriptorPool> shadowMapDescriptorPools;

  DescriptorSets colorDescriptorSets;
  DescriptorSets zPrePassDescriptorSets;
  VkDescriptorSetLayout shadowMapDescriptorSetLayout;
  std::unordered_map<uint32_t, DescriptorSets> shadowMapDescriptorSets;

  void CreateColorDescriptorPool(const VkDevice& device, Render& render,
                                 size_t textureNum);
  void CreateZPrePassDescriptorPool(const VkDevice& device,
                                    const Render& render);
  void CreateShadowMapDescriptorPool(const VkDevice& device,
                                     const Render& render,
                                     VkDescriptorPool& descriptorPool);

  void CreateColorDescriptorSets(
      const VkDevice& device, Render& render,
      const VkDescriptorSetLayout& colorDescriptorSetLayout,
      const std::vector<Texture>& textures);
  void CreateZPrePassDescriptorSets(
      const VkDevice& device, const Render& render,
      const VkDescriptorSetLayout& zPrePassDescriptorSetLayout);
  void CreateShadowMapDescriptorSets(
      const VkDevice& device, const Render& render,
      const VkDescriptorSetLayout& descriptorSetLayout,
      const VkDescriptorPool& descriptorPool,
      const ShadowMapBuffer& shadowMapBuffer, DescriptorSets& descriptorSets);

  void UpdateBufferPointers();
  void CreateUniformBuffer(const Device& device, Render& render);
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
  void UpdateColorDescriptorSets(const VkDevice& device, Render& render);
  void UpdateUniformBuffer(const uint32_t currentImage);
  void UpdateShadowMapUniformBuffers(
      const Device& device, const Render& render, const uint32_t currentImage,
      std::unordered_map<int, std::weak_ptr<BaseLight>>& lightsById);

  [[nodiscard]] const DescriptorSets& GetShadowMapDescriptorSetsByIndex(
      const uint32_t index) {
    return shadowMapDescriptorSets[index];
  }
  [[nodiscard]] const VkDescriptorSet& GetShadowMapDescriptorSetByIndices(
      const uint32_t lightId, const uint32_t currentFrame) {
    return shadowMapDescriptorSets[lightId][currentFrame];
  }

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

  void CreateDescriptor(
      const Device& device, Render& render,
      const std::vector<Texture>& textures,
      const VkDescriptorSetLayout& colorDescriptorSetLayout,
      const VkDescriptorSetLayout& zPrePassDescriptorSetLayout,
      const VkDescriptorSetLayout& shadowMapDescriptorSetLayout);
  void DestroyDesciptor(const VkDevice& device, const Render& render);

  ShadowMapBuffer& CreateShadowMapUniformBufferByIndex(const Device& device,
                                                       const Render& render,
                                                       const uint32_t lightId);
  void RemoveShadowMapUniformBufferByIndex(const VkDevice& device,
                                           const Render& render,
                                           const uint32_t lightId);
  const VkDescriptorPool& CreateShadowMapDescriptorPoolByIndex(
      const VkDevice& device, const Render& render, const uint32_t lightId);
  void RemoveShadowMapDescriptorPoolByIndex(const VkDevice& device,
                                            const uint32_t lightId);
  const DescriptorSets& CreateShadowMapDescriptorSetsByIndex(
      const Device& device, const Render& render, const uint32_t lightId);
  const VkDescriptorSet& CreateShadowMapDescriptorSetByIndices(
      const Device& device, const Render& render, const uint32_t lightId,
      const uint32_t currentFrame);
  void RemoveShadowMapDescriptorSetsByIndex(const VkDevice& device,
                                            const uint32_t lightId);
};

#undef DEFINE_GET_DESCRIPTOR_SET