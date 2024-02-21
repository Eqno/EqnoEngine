#pragma once

#include <vulkan/vulkan_core.h>

#include <string>

#include "stb_image.h"
#include "utils.h"

class Device;
class Render;

class Texture {
  VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;

  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;

  void CreateTextureImage(const Device& device, const Render& render,
                          int texWidth, int texHeight, int texChannels,
                          stbi_uc* pixels);
  void CreateTextureImageView(const VkDevice& device);
  void CreateTextureSampler(const Device& device);

  static void CopyBufferToImage(const Device& device, const Render& render,
                                VkBuffer buffer, VkImage image, uint32_t width,
                                uint32_t height);

 public:
  Texture() = default;

  Texture(const Device& device, const Render& render, const int width,
          const int height, const int channels, stbi_uc* data) {
    CreateTexture(device, render, width, height, channels, data);
  }

  Texture(VkFormat imageFormat, const Device& device, const Render& render,
          const int width, const int height, const int channels, stbi_uc* data)
      : imageFormat(imageFormat) {
    CreateTexture(device, render, width, height, channels, data);
  }

  [[nodiscard]] const VkImageView& GetTextureImageView() const {
    return textureImageView;
  }

  [[nodiscard]] VkSampler GetTextureSampler() const { return textureSampler; }

  static VkImageView CreateImageView(const VkDevice& device, VkImage image,
                                     VkFormat format,
                                     VkImageAspectFlags aspectFlags);

  static void CreateImage(const Device& device, uint32_t width, uint32_t height,
                          VkFormat format, VkImageTiling tiling,
                          VkImageUsageFlags usage,
                          VkMemoryPropertyFlags properties, VkImage& image,
                          VkDeviceMemory& imageMemory);

  static void TransitionImageLayout(const Device& device, const Render& render,
                                    const VkImage& image, const VkFormat format,
                                    const VkImageLayout oldLayout,
                                    const VkImageLayout newLayout);

  void CreateTexture(const Device& device, const Render& render, int width,
                     int height, int channels, stbi_uc* data);
  void DestroyTexture(const VkDevice& device) const;
};
