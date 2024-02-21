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

 public:
  VkFormat GetDepthFormat() const { return depthFormat; }

  [[nodiscard]] VkImage& GetDepthImage() { return depthImage; }
  [[nodiscard]] VkImageView& GetDepthImageView() { return depthImageView; }

  void CreateDepthResources(const Device& device, const uint32_t imageWidth,
                            const uint32_t imageHeight,
                            const VkImageUsageFlags usage);
  void TransitionDepthImageLayout(const Device& device, const Render& render,
                                  VkImageLayout oldLayout,
                                  VkImageLayout newLayout);

  static VkFormat FindSupportedFormat(const VkPhysicalDevice& device,
                                      const std::vector<VkFormat>& candidates,
                                      VkImageTiling tiling,
                                      VkFormatFeatureFlags features);

  static VkFormat FindDepthFormat(const VkPhysicalDevice& device);
  static bool HasStencilComponent(VkFormat format);
  void DestroyDepthResource(const VkDevice& device) const;
};
