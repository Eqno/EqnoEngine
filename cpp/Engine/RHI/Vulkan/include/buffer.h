#pragma once

#include <vulkan/vulkan_core.h>

#include <glm/fwd.hpp>
#include <vector>

#include "base.h"
#include "vertex.h"

class Device;
class Render;

class Buffer : public Base {
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
