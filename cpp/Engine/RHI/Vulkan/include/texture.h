#pragma once

#include <Engine/RHI/Vulkan/include/utils.h>
#include <stb_image.h>
#include <vulkan/vulkan_core.h>

class Device;
class Render;

class Texture {
  bool createInterrupted = false;
  VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;

  uint32_t mipLevels;
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;

  void CreateTextureImage(const Device& device, const Render& render,
                          int texWidth, int texHeight, int texChannels,
                          std::weak_ptr<TextureDataContent> pixelsPtr);
  void CreateTextureImageView(const VkDevice& device);
  void CreateTextureSampler(const Device& device);

  static void CopyBufferToImage(const Device& device, const Render& render,
                                VkBuffer buffer, VkImage image, uint32_t width,
                                uint32_t height);

 public:
  Texture() = default;

  Texture(const Device& device, const Render& render, const int width,
          const int height, const int channels,
          std::weak_ptr<TextureDataContent> data) {
    CreateTexture(device, render, width, height, channels, data);
  }

  Texture(VkFormat imageFormat, const Device& device, const Render& render,
          const int width, const int height, const int channels,
          std::weak_ptr<TextureDataContent> data)
      : imageFormat(imageFormat) {
    CreateTexture(device, render, width, height, channels, data);
  }

  bool GetCreateInterrupted() const { return createInterrupted; }
  [[nodiscard]] const VkImageView& GetTextureImageView() const {
    return textureImageView;
  }
  [[nodiscard]] VkSampler GetTextureSampler() const { return textureSampler; }

  static std::pair<VkImage, VkDeviceMemory> CreateImage(
      const Device& device, uint32_t width, uint32_t height,
      const uint32_t mipLevels, VkSampleCountFlagBits numSamples,
      VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
      VkMemoryPropertyFlags properties);
  static VkImageView CreateImageView(const VkDevice& device, VkImage image,
                                     const uint32_t mipLevels, VkFormat format,
                                     VkImageAspectFlags aspectFlags);
  static VkSampler CreateSampler(const Device& device, const uint32_t mipLevels,
                                 VkBool32 anisotropyEnable,
                                 VkBool32 compareEnable, VkCompareOp compareOp);
  static void GenerateMipmaps(const Device& device, const Render& render,
                              const VkImage image, const int32_t width,
                              const int32_t height, const uint32_t mipLevels,
                              const VkFormat imageFormat);
  static void TransitionImageLayout(
      const Device& device, const Render& render, const VkImage& image,
      const uint32_t mipLevels, const VkFormat format,
      const VkImageLayout oldLayout, const VkImageLayout newLayout,
      const VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

  void CreateTexture(const Device& device, const Render& render, int width,
                     int height, int channels,
                     std::weak_ptr<TextureDataContent> data);
  void DestroyTexture(const VkDevice& device) const;
};
