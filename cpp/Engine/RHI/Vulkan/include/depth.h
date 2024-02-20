#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>

#include "base.h"

class Device;
class SwapChain;

class Depth : public Base {
  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;

 public:
  [[nodiscard]] const VkImageView& GetDepthImageView() const {
    return depthImageView;
  }

  void CreateDepthResources(const Device& device, const uint32_t imageWidth,
                            const uint32_t imageHeight);

  static VkFormat FindSupportedFormat(const VkPhysicalDevice& device,
                                      const std::vector<VkFormat>& candidates,
                                      VkImageTiling tiling,
                                      VkFormatFeatureFlags features);

  static VkFormat FindDepthFormat(const VkPhysicalDevice& device);
  static bool HasStencilComponent(VkFormat format);
  void DestroyDepthResource(const VkDevice& device) const;
};
