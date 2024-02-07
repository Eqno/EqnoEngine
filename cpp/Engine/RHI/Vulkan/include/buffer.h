#pragma once

#include <vulkan/vulkan_core.h>

#include <glm/fwd.hpp>
#include <vector>

#include "base.h"
#include "vertex.h"

class Device;
class Render;

using UniformMapped = std::vector<void*>;
using UniformBuffers = std::vector<VkBuffer>;
using UniformMemories = std::vector<VkDeviceMemory>;

class DataBuffer : public Base {
  VkBuffer vertexBuffer{};
  VkDeviceMemory vertexBufferMemory{};

  VkBuffer indexBuffer{};
  VkDeviceMemory indexBufferMemory{};

  void CreateVertexBuffer(const Device& device,
                          const std::vector<Vertex>& vertices,
                          const Render& render);
  void CreateIndexBuffer(const Device& device,
                         const std::vector<uint32_t>& indices,
                         const Render& render);

 public:
  static uint32_t MemoryType(const VkPhysicalDevice& physicalDevice,
                             const glm::uint32_t& typeFilter,
                             const VkMemoryPropertyFlags& properties);

  static void CreateBuffer(const Device& device, const VkDeviceSize& size,
                           const VkBufferUsageFlags& usage,
                           const VkMemoryPropertyFlags& properties,
                           VkBuffer& buffer, VkDeviceMemory& bufferMemory);

  void CreateBuffers(const Device& device, const std::vector<Vertex>& vertices,
                     const std::vector<uint32_t>& indices,
                     const Render& render);
  void DestroyBuffers(const VkDevice& device) const;

  [[nodiscard]] const VkBuffer& GetVertexBuffer() const { return vertexBuffer; }

  [[nodiscard]] const VkDeviceMemory& GetVertexBufferMemory() const {
    return vertexBufferMemory;
  }

  [[nodiscard]] const VkBuffer& GetIndexBuffer() const { return indexBuffer; }

  [[nodiscard]] const VkDeviceMemory& GetIndexBufferMemory() const {
    return indexBufferMemory;
  }
};

class UniformBuffer : public Base {
 protected:
  UniformBuffers uniformBuffers;
  UniformMemories uniformBuffersMemory;
  UniformMapped uniformBuffersMapped;

 public:
  [[nodiscard]] const UniformBuffers& GetUniformBuffers() const {
    return uniformBuffers;
  }
  [[nodiscard]] const VkBuffer& GetUniformBufferByIndex(
      const size_t index) const {
    return uniformBuffers[index];
  }
  void CreateUniformBuffer(const Device& device, const Render& render,
                           unsigned long long bufferSize);
  void DestroyUniformBuffer(const VkDevice& device, const Render& render) const;
};

class CameraBuffer : public UniformBuffer {
 public:
  void UpdateUniformBuffer(BaseCamera* camera, const uint32_t currentImage);
};

class MaterialBuffer : public UniformBuffer {
 public:
  void UpdateUniformBuffer(BaseMaterial* material, const uint32_t currentImage);
};

class LightChannelBuffer : public UniformBuffer {
 public:
  void UpdateUniformBuffer(LightChannel* lightChannel,
                           const uint32_t currentImage);
};

class BufferManager {
#define DefineUniformBuffer(type, data, upper, lower)                        \
 private:                                                                    \
  std::unordered_map<type*, std::pair<int, upper##Buffer>> lower##Buffers;   \
                                                                             \
 public:                                                                     \
  upper##Buffer& Create##upper##Buffer(type* lower, const Device& device,    \
                                       const Render& render) {               \
    auto [iter, inserted] =                                                  \
        lower##Buffers.insert({lower, {1, upper##Buffer()}});                \
    if (inserted == true) {                                                  \
      iter->second.second.CreateUniformBuffer(device, render, sizeof(data)); \
    } else {                                                                 \
      iter->second.first++;                                                  \
    }                                                                        \
    return iter->second.second;                                              \
  }                                                                          \
  void Destroy##upper##Buffer(type* lower, const VkDevice& device,           \
                              const Render& render) {                        \
    auto iter = lower##Buffers.find(lower);                                  \
    if (iter == lower##Buffers.end()) {                                      \
      return;                                                                \
    }                                                                        \
    iter->second.first--;                                                    \
    if (iter->second.first <= 0) {                                           \
      iter->second.second.DestroyUniformBuffer(device, render);              \
      lower##Buffers.erase(iter);                                            \
    }                                                                        \
  }
  DefineUniformBuffer(LightChannel, LightsData, LightChannel, lightChannel)
      DefineUniformBuffer(BaseMaterial, MaterialData, Material, material)
          DefineUniformBuffer(BaseCamera, CameraData, Camera, camera)
};
