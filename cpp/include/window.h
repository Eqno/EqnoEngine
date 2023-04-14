#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <utility>
#include <vulkan/vulkan_core.h>

struct GLFWwindow;

class Window {
	VkSurfaceKHR surface {};
	bool         frameBufferResized = false;
public:
	GLFWwindow* window = nullptr;

	[[nodiscard]] auto GetSurface() const -> const VkSurfaceKHR& {
		return surface;
	}

	[[nodiscard]] auto GetFrameBufferSize() const -> std::pair<int, int>;

	[[nodiscard]] auto GetFrameBufferResized() const -> const bool& {
		return frameBufferResized;
	}

	[[nodiscard]] static auto
	GetRequiredExtensions() -> std::pair<const char**, uint32_t>;

	auto SetFrameBufferResized(bool resized) -> void {
		frameBufferResized = resized;
	}

	auto CreateWindow() -> void;
	auto CreateWindow(int width, int height) -> void;
	auto CreateSurface(const VkInstance& instance) -> void;

	auto DestroyWindow() const -> void;

	auto OnRecreateSwapChain() const -> void;
};
