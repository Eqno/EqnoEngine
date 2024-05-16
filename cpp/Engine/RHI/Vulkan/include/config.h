#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace VulkanConfig {
constexpr int MAX_FRAMES_IN_FLIGHT = 3;
constexpr bool ENABLE_VALIDATION_LAYER = false;

constexpr int DEFAULT_WINDOW_WIDTH = 800;
constexpr int DEFAULT_WINDOW_HEIGHT = 600;

const std::vector DEVICE_EXTENSIONS{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}  // namespace VulkanConfig
