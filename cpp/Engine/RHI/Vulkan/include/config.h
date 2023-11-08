#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace VulkanConfig {
	constexpr int MAX_FRAMES_IN_FLIGHT = 2;
	constexpr bool ENABLE_VALIDATION_LAYER = true;

	constexpr int DEFAULT_WINDOW_WIDTH = 800;
	constexpr int DEFAULT_WINDOW_HEIGHT = 600;

	const std::string ROOT_PATH = "./";
	const std::string ENGINE_PATH = ROOT_PATH + "Engine/";
	const std::string GAMES_PATH = ROOT_PATH + "Games/";

	const std::string SHADER_PATH = GAMES_PATH + "Test/Assets/Shaders/glsl/";

	const std::string MODEL_PATH = GAMES_PATH +
		"Test/Assets/Models/res/viking_room.obj";
	const std::string TEXTURE_PATH = GAMES_PATH +
		"Test/Assets/Textures/viking_room.png";

	const std::vector DEVICE_EXTENSIONS {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}
