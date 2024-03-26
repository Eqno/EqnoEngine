#include <Engine/RHI/Vulkan/include/buffer.h>
#include <Engine/RHI/Vulkan/include/device.h>
#include <Engine/RHI/Vulkan/include/render.h>
#include <Engine/RHI/Vulkan/include/texture.h>

#include <stdexcept>

std::pair<VkImage, VkDeviceMemory> Texture::CreateImage(
    const Device& device, const uint32_t width, const uint32_t height,
    const uint32_t mipLevels, VkSampleCountFlagBits numSamples,
    const VkFormat format, const VkImageTiling tiling,
    const VkImageUsageFlags usage, const VkMemoryPropertyFlags properties) {
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
      .mipLevels = mipLevels,
      .arrayLayers = 1,
      .samples = numSamples,
      .tiling = tiling,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };
  VkImage image;
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
  VkDeviceMemory imageMemory;
  if (vkAllocateMemory(device.GetLogical(), &allocInfo, nullptr,
                       &imageMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }
  vkBindImageMemory(device.GetLogical(), image, imageMemory, 0);
  return {image, imageMemory};
}

VkImageView Texture::CreateImageView(const VkDevice& device,
                                     const VkImage image,
                                     const uint32_t mipLevels,
                                     const VkFormat format,
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
              .levelCount = mipLevels,
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

VkSampler Texture::CreateSampler(const Device& device, const uint32_t mipLevels,
                                 VkBool32 anisotropyEnable,
                                 VkBool32 compareEnable,
                                 VkCompareOp compareOp) {
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(device.GetPhysical(), &properties);

  const VkSamplerCreateInfo samplerInfo{
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .mipLodBias = 0,
      .anisotropyEnable = anisotropyEnable,
      .maxAnisotropy = anisotropyEnable == VK_TRUE
                           ? properties.limits.maxSamplerAnisotropy
                           : 1,
      .compareEnable = compareEnable,
      .compareOp = compareOp,
      .minLod = 0,
      .maxLod = static_cast<float>(mipLevels),
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE,
  };
  VkSampler sampler;
  if (vkCreateSampler(device.GetLogical(), &samplerInfo, nullptr, &sampler) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create texture sampler!");
  }
  return sampler;
}

void Texture::GenerateMipmaps(const Device& device, const Render& render,
                              const VkImage image, const int32_t width,
                              const int32_t height, const uint32_t mipLevels,
                              const VkFormat imageFormat) {
  VkFormatProperties formatProperties;
  vkGetPhysicalDeviceFormatProperties(device.GetPhysical(), imageFormat,
                                      &formatProperties);

  if (!(formatProperties.optimalTilingFeatures &
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    throw std::runtime_error(
        "texture image format does not support linear blitting!");
  }

  VkCommandBuffer commandBuffer;
  render.BeginSingleTimeCommands(device.GetLogical(), &commandBuffer);

  VkImageMemoryBarrier barrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };
  int32_t mipWidth = width;
  int32_t mipHeight = height;
  for (uint32_t i = 1; i < mipLevels; i++) {
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &barrier);

    VkImageBlit blit{
        .srcSubresource =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i - 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .srcOffsets =
            {
                {0, 0, 0},
                {mipWidth, mipHeight, 1},
            },
        .dstSubresource =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .dstOffsets =
            {
                {0, 0, 0},
                {mipWidth > 1 ? mipWidth / 2 : 1,
                 mipHeight > 1 ? mipHeight / 2 : 1, 1},
            },
    };
    vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                   VK_FILTER_LINEAR);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                         0, nullptr, 1, &barrier);
    if (mipWidth > 1) mipWidth /= 2;
    if (mipHeight > 1) mipHeight /= 2;
  }
  barrier.subresourceRange.baseMipLevel = mipLevels - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);
  render.EndSingleTimeCommands(device, &commandBuffer);
}

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
  auto [image, imageMemory] = CreateImage(
      device, texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT,
      imageFormat, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
          VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  textureImage = image;
  textureImageMemory = imageMemory;

  TransitionImageLayout(device, render, textureImage, mipLevels, imageFormat,
                        VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  CopyBufferToImage(device, render, stagingBuffer, textureImage,
                    static_cast<uint32_t>(texWidth),
                    static_cast<uint32_t>(texHeight));
  vkDestroyBuffer(device.GetLogical(), stagingBuffer, nullptr);
  vkFreeMemory(device.GetLogical(), stagingBufferMemory, nullptr);

  if (render.GetEnableMipmap()) {
    GenerateMipmaps(device, render, image, texWidth, texHeight, mipLevels,
                    imageFormat);
  } else {
    TransitionImageLayout(device, render, textureImage, mipLevels, imageFormat,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }
}

void Texture::CreateTextureImageView(const VkDevice& device) {
  textureImageView = CreateImageView(device, textureImage, mipLevels,
                                     imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Texture::CreateTextureSampler(const Device& device) {
  textureSampler =
      CreateSampler(device, mipLevels, VK_TRUE, VK_FALSE, VK_COMPARE_OP_ALWAYS);
}

void Texture::TransitionImageLayout(
    const Device& device, const Render& render, const VkImage& image,
    const uint32_t mipLevels, const VkFormat format,
    const VkImageLayout oldLayout, const VkImageLayout newLayout,
    const VkImageAspectFlags aspectMask, VkCommandBuffer commandBuffer) {
  bool requireOneTimeCommandBuffer = false;
  if (commandBuffer == VK_NULL_HANDLE) {
    requireOneTimeCommandBuffer = true;

    commandBuffer = {};
    render.BeginSingleTimeCommands(device.GetLogical(), &commandBuffer);
  }

  VkImageMemoryBarrier barrier{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                               .oldLayout = oldLayout,
                               .newLayout = newLayout,
                               .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                               .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                               .image = image,
                               .subresourceRange = {
                                   .baseMipLevel = 0,
                                   .levelCount = mipLevels,
                                   .baseArrayLayer = 0,
                                   .layerCount = 1,
                               }};

  barrier.subresourceRange.aspectMask = aspectMask;
  if (aspectMask == (aspectMask | VK_IMAGE_ASPECT_DEPTH_BIT) &&
      Depth::HasStencilComponent(format)) {
    barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
  }

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
             newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                  VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                       VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }
  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);

  if (requireOneTimeCommandBuffer == true) {
    render.EndSingleTimeCommands(device, &commandBuffer);
  }
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
  if (render.GetEnableMipmap()) {
    mipLevels =
        static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) +
        1;
  } else {
    mipLevels = 1;
  }
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
