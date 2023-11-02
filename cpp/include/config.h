#pragma once

#include <vector>

namespace Config {
	constexpr int MAX_FRAMES_IN_FLIGHT = 2;
	constexpr bool ENABLE_VALIDATION_LAYER = true;

	constexpr uint32_t DEFAULT_WINDOW_WIDTH = 800;
	constexpr uint32_t DEFAULT_WINDOW_HEIGHT = 600;

	const std::vector DEVICE_EXTENSIONS {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}
