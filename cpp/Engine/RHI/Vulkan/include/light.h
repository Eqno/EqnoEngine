#pragma once

#include <vulkan/vulkan_core.h>

#include "uniform.h"

class Light {
  UniformBuffers uniformBuffers;
  UniformMemories uniformBuffersMemory;
  UniformMapped uniformBuffersMapped;

 public:
  Light() = default;

  const VkBuffer& GetUniformBufferByIndex(size_t index) {
    return uniformBuffers[index];
  }
  void CreateUniformBuffers(const Device& device, int maxFramesInFlight);
  void DestroyUniformBuffers(const VkDevice& device,
                             int maxFramesInFlight) const;
};
