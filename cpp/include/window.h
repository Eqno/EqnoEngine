#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <utility>
#include <vulkan/vulkan_core.h>

struct GLFWwindow;

class Window {
	VkSurfaceKHR surface {};
	bool frameBufferResized = false;

public:
	GLFWwindow* window = nullptr;

	[[nodiscard]] const VkSurfaceKHR& GetSurface() const {
		return surface;
	}

	[[nodiscard]] std::pair<int, int> GetFrameBufferSize() const;

	[[nodiscard]] const bool& GetFrameBufferResized() const {
		return frameBufferResized;
	}

	[[nodiscard]] static std::pair<const char**, uint32_t>
	GetRequiredExtensions();

	void SetFrameBufferResized(const bool resized) {
		frameBufferResized = resized;
	}

	void CreateWindow();
	void CreateWindow(int width, int height);
	void CreateSurface(const VkInstance& instance);

	void DestroyWindow() const;

	void OnRecreateSwapChain() const;
};
