#include "../include/light.h"

#include <Engine/Utility/include/TypeUtils.h>

#include "../include/buffer.h"
#include "../include/device.h"

void Light::CreateUniformBuffers(const Device& device, int maxFramesInFlight) {
  for (int i = 0; i < maxFramesInFlight; i++) {
    Buffer::CreateBuffer(device, sizeof(LightData),
                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         uniformBuffers[i], uniformBuffersMemory[i]);
    vkMapMemory(device.GetLogical(), uniformBuffersMemory[i], 0,
                sizeof(LightData), 0, &uniformBuffersMapped[i]);
  }
}
void Light::DestroyUniformBuffers(const VkDevice& device,
                                  int maxFramesInFlight) const {
  for (auto i = 0; i < maxFramesInFlight; i++) {
    vkDestroyBuffer(device, uniformBuffers[i], nullptr);
    vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
  }
}