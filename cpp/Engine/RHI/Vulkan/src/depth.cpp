#include "../include/depth.h"

#include <stdexcept>

#include "../include/device.h"
#include "../include/render.h"
#include "../include/swapchain.h"
#include "../include/texture.h"

void Depth::CreateDepthResources(const Device& device,
                                 const uint32_t imageWidth,
                                 const uint32_t imageHeight,
                                 VkSampleCountFlagBits numSamples,
                                 const VkImageUsageFlags usage) {
  depthFormat = FindDepthFormat(device.GetPhysical());
  auto [image, imageMemory] = Texture::CreateImage(
      device, imageWidth, imageHeight, 1, numSamples, depthFormat,
      VK_IMAGE_TILING_OPTIMAL, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  depthImage = image;
  depthImageMemory = imageMemory;
  depthImageView =
      Texture::CreateImageView(device.GetLogical(), depthImage, 1, depthFormat,
                               VK_IMAGE_ASPECT_DEPTH_BIT);
  if (usage == (usage | VK_IMAGE_USAGE_SAMPLED_BIT)) {
    depthSampler = Texture::CreateSampler(device, 1, VK_FALSE, VK_TRUE,
                                          VK_COMPARE_OP_LESS);
  }
}
void Depth::TransitionDepthImageLayout(const Device& device,
                                       const Render& render,
                                       VkImageLayout oldLayout,
                                       VkImageLayout newLayout,
                                       VkCommandBuffer commandBuffer) {
  Texture::TransitionImageLayout(device, render, depthImage, 1, depthFormat,
                                 oldLayout, newLayout,
                                 VK_IMAGE_ASPECT_DEPTH_BIT, commandBuffer);
}

VkFormat Depth::FindSupportedFormat(const VkPhysicalDevice& device,
                                    const std::vector<VkFormat>& candidates,
                                    const VkImageTiling tiling,
                                    const VkFormatFeatureFlags features) {
  for (const VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(device, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    }
    if (tiling == VK_IMAGE_TILING_OPTIMAL &&
        (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw std::runtime_error("failed to find supported format!");
}

VkFormat Depth::FindDepthFormat(const VkPhysicalDevice& device) {
  return FindSupportedFormat(
      device,
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
       VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool Depth::HasStencilComponent(const VkFormat format) {
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
         format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void Depth::DestroyDepthResource(const VkDevice& device) const {
  if (depthSampler != VK_NULL_HANDLE) {
    vkDestroySampler(device, depthSampler, nullptr);
  }
  vkDestroyImageView(device, depthImageView, nullptr);
  vkDestroyImage(device, depthImage, nullptr);
  vkFreeMemory(device, depthImageMemory, nullptr);
}
