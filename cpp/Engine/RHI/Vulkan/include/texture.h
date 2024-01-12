#pragma once

#include <vulkan/vulkan_core.h>

#include <string>

#include "stb_image.h"
#include "utils.h"

class Device;
class Render;

class Texture {
  VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;

  VkImage textureImage{};
  VkDeviceMemory textureImageMemory{};
  VkImageView textureImageView{};
  VkSampler textureSampler{};

  void CreateTextureImage(const Device& device, const Render& render,
                          int texWidth, int texHeight, int texChannels,
                          stbi_uc* pixels);
  void CreateTextureImageView(const VkDevice& device);
  void CreateTextureSampler(const Device& device);

  static void TransitionImageLayout(const Device& device, const Render& render,
                                    VkImage image, VkImageLayout oldLayout,
                                    VkImageLayout newLayout);
  static void CopyBufferToImage(const Device& device, const Render& render,
                                VkBuffer buffer, VkImage image, uint32_t width,
                                uint32_t height);

 public:
  Texture() = default;

  explicit Texture(const std::string& imageFormat)
      : imageFormat(VulkanUtils::ParseImageFormat(imageFormat)) {}

  Texture(const Device& device, const Render& render, const int width,
          const int height, const int channels, stbi_uc* data) {
    CreateTexture(device, render, width, height, channels, data);
  }

  Texture(const std::string& imageFormat, const Device& device,
          const Render& render, const int width, const int height,
          const int channels, stbi_uc* data)
      : imageFormat(VulkanUtils::ParseImageFormat(imageFormat)) {
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

  void CreateTexture(const Device& device, const Render& render, int width,
                     int height, int channels, stbi_uc* data);
  void DestroyTexture(const VkDevice& device) const;
};
