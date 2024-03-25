#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>

#include "base.h"

class Device;
class Render;
class SwapChain;

class Depth : public Base {
  VkFormat depthFormat;

  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;
  VkSampler depthSampler = VK_NULL_HANDLE;

 public:
  VkFormat GetDepthFormat() const { return depthFormat; }

  [[nodiscard]] const VkImage& GetDepthImage() { return depthImage; }
  [[nodiscard]] const VkSampler& GetDepthSampler() { return depthSampler; }
  [[nodiscard]] const VkImageView& GetDepthImageView() {
    return depthImageView;
  }

  void CreateDepthResources(const Device& device, const uint32_t imageWidth,
                            const uint32_t imageHeight,
                            VkSampleCountFlagBits numSamples,
                            const VkImageUsageFlags usage);
  void TransitionDepthImageLayout(
      const Device& device, const Render& render, VkImageLayout oldLayout,
      VkImageLayout newLayout, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

  static VkFormat FindSupportedFormat(const VkPhysicalDevice& device,
                                      const std::vector<VkFormat>& candidates,
                                      VkImageTiling tiling,
                                      VkFormatFeatureFlags features);

  static VkFormat FindDepthFormat(const VkPhysicalDevice& device);
  static bool HasStencilComponent(VkFormat format);
  void DestroyDepthResource(const VkDevice& device) const;
};
