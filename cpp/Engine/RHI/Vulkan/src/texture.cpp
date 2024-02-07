#include "../include/texture.h"

#include <stdexcept>

#include "../include/buffer.h"
#include "../include/device.h"
#include "../include/render.h"

void Texture::CreateTextureImage(const Device& device, const Render& render,
                                 const int texWidth, const int texHeight,
                                 const int texChannels, stbi_uc* pixels) {
  const VkDeviceSize imageSize =
      static_cast<VkDeviceSize>(texWidth) * texHeight * 4;

  if (!pixels) {
    throw std::runtime_error("texture pixels invalid!");
  }

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  DataBuffer::CreateBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           stagingBuffer, stagingBufferMemory);

  void* imageData = nullptr;
  vkMapMemory(device.GetLogical(), stagingBufferMemory, 0, imageSize, 0,
              &imageData);
  memcpy(imageData, pixels, imageSize);
  vkUnmapMemory(device.GetLogical(), stagingBufferMemory);

  stbi_image_free(pixels);
  CreateImage(device, texWidth, texHeight, imageFormat, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage,
              textureImageMemory);

  TransitionImageLayout(device, render, textureImage, VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  CopyBufferToImage(device, render, stagingBuffer, textureImage,
                    static_cast<uint32_t>(texWidth),
                    static_cast<uint32_t>(texHeight));
  TransitionImageLayout(device, render, textureImage,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(device.GetLogical(), stagingBuffer, nullptr);
  vkFreeMemory(device.GetLogical(), stagingBufferMemory, nullptr);
}

void Texture::CreateTextureImageView(const VkDevice& device) {
  textureImageView = CreateImageView(device, textureImage, imageFormat,
                                     VK_IMAGE_ASPECT_COLOR_BIT);
}

void Texture::CreateTextureSampler(const Device& device) {
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(device.GetPhysical(), &properties);

  const VkSamplerCreateInfo samplerInfo{
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .anisotropyEnable = VK_TRUE,
      .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE,
  };

  if (vkCreateSampler(device.GetLogical(), &samplerInfo, nullptr,
                      &textureSampler) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture sampler!");
  }
}

VkImageView Texture::CreateImageView(const VkDevice& device,
                                     const VkImage image, const VkFormat format,
                                     const VkImageAspectFlags aspectFlags) {
  const VkImageViewCreateInfo viewInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = format,
      .subresourceRange =
          {
              .aspectMask = aspectFlags,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };
  VkImageView imageView;
  if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image view!");
  }
  return imageView;
}

void Texture::CreateImage(const Device& device, const uint32_t width,
                          const uint32_t height, const VkFormat format,
                          const VkImageTiling tiling,
                          const VkImageUsageFlags usage,
                          const VkMemoryPropertyFlags properties,
                          VkImage& image, VkDeviceMemory& imageMemory) {
  const VkImageCreateInfo imageInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = format,
      .extent =
          {
              .width = width,
              .height = height,
              .depth = 1,
          },
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = tiling,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  if (vkCreateImage(device.GetLogical(), &imageInfo, nullptr, &image) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device.GetLogical(), image, &memRequirements);

  const VkMemoryAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex = DataBuffer::MemoryType(
          device.GetPhysical(), memRequirements.memoryTypeBits, properties),
  };

  if (vkAllocateMemory(device.GetLogical(), &allocInfo, nullptr,
                       &imageMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }
  vkBindImageMemory(device.GetLogical(), image, imageMemory, 0);
}

void Texture::TransitionImageLayout(const Device& device, const Render& render,
                                    const VkImage image,
                                    const VkImageLayout oldLayout,
                                    const VkImageLayout newLayout) {
  VkCommandBuffer commandBuffer;
  render.BeginSingleTimeCommands(device.GetLogical(), &commandBuffer);

  VkImageMemoryBarrier barrier{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                               .oldLayout = oldLayout,
                               .newLayout = newLayout,
                               .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                               .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                               .image = image,
                               .subresourceRange = {
                                   .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                   .baseMipLevel = 0,
                                   .levelCount = 1,
                                   .baseArrayLayer = 0,
                                   .layerCount = 1,
                               }};

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);
  render.EndSingleTimeCommands(device, &commandBuffer);
}

void Texture::CopyBufferToImage(const Device& device, const Render& render,
                                const VkBuffer buffer, const VkImage image,
                                const uint32_t width, const uint32_t height) {
  VkCommandBuffer commandBuffer;
  render.BeginSingleTimeCommands(device.GetLogical(), &commandBuffer);

  const VkBufferImageCopy region{
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .mipLevel = 0,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
      .imageOffset = {0, 0, 0},
      .imageExtent = {width, height, 1},
  };

  vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
  render.EndSingleTimeCommands(device, &commandBuffer);
}

void Texture::CreateTexture(const Device& device, const Render& render,
                            const int width, const int height,
                            const int channels, stbi_uc* data) {
  CreateTextureImage(device, render, width, height, channels, data);
  CreateTextureImageView(device.GetLogical());
  CreateTextureSampler(device);
}

void Texture::DestroyTexture(const VkDevice& device) const {
  vkDestroySampler(device, textureSampler, nullptr);
  vkDestroyImageView(device, textureImageView, nullptr);
  vkDestroyImage(device, textureImage, nullptr);
  vkFreeMemory(device, textureImageMemory, nullptr);
}
