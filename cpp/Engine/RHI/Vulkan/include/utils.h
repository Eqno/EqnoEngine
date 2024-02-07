#pragma once

#include <Engine/Utility/include/TypeUtils.h>
#include <assimp/scene.h>
#include <vulkan/vulkan_core.h>

class Vertex;

namespace VulkanUtils {
inline std::unordered_map<TextureType, VkFormat> TextureFormat{
    {TextureType::BaseColor, VK_FORMAT_R8G8B8A8_SRGB},
    {TextureType::Roughness, VK_FORMAT_R8G8B8A8_UNORM},
    {TextureType::Metallic, VK_FORMAT_R8G8B8A8_UNORM},
    {TextureType::Normal, VK_FORMAT_R8G8B8A8_UNORM},
    {TextureType::AO, VK_FORMAT_R8G8B8A8_UNORM},
};

inline VkFormat ParseImageFormat(const std::string& imageFormat) {
  if (imageFormat == "RGBA_SRGB") {
    return VK_FORMAT_R8G8B8A8_SRGB;
  }
  if (imageFormat == "RGBA_UNORM") {
    return VK_FORMAT_R8G8B8A8_UNORM;
  }
  if (imageFormat == "A_UNORM") {
    return VK_FORMAT_A8_UNORM_KHR;
  }
  return VK_FORMAT_R8G8B8A8_UNORM;
}

inline VkColorSpaceKHR ParseColorSpace(const std::string& colorSpace) {
  if (colorSpace == "SRGB_NONLINEAR") {
    return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  }
  if (colorSpace == "SRGB_LINEAR") {
    return VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT;
  }
  return VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT;
}
}  // namespace VulkanUtils