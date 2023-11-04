#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Config {
	constexpr int MAX_FRAMES_IN_FLIGHT = 2;
	constexpr bool ENABLE_VALIDATION_LAYER = true;

	constexpr uint32_t DEFAULT_WINDOW_WIDTH = 800;
	constexpr uint32_t DEFAULT_WINDOW_HEIGHT = 600;

	const std::string MODEL_PATH = "models/viking_room.obj";
	const std::string TEXTURE_PATH = "textures/viking_room.png";

	const std::vector<const char*> DEVICE_EXTENSIONS {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}
