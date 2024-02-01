#pragma once

#include <assimp/scene.h>
#include <vulkan/vulkan_core.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <string>

class Vertex;

namespace VulkanUtils {
inline VkFormat ParseImageFormat(const std::string& imageFormat) {
  if (imageFormat == "SRGB") {
    return VK_FORMAT_R8G8B8A8_SRGB;
  }
  return VK_FORMAT_R8G8B8A8_UNORM;
}

inline VkColorSpaceKHR ParseColorSpace(const std::string& colorSpace) {
  if (colorSpace == "SRGB") {
    return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  }
  return VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT;
}
}  // namespace VulkanUtils