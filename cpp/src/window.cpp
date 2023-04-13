#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

#include "window.h"
#include "config.h"

static auto FrameBufferResizeCallback(
	[[maybe_unused]] GLFWwindow* window,
	[[maybe_unused]] int         width,
	[[maybe_unused]] int         height
) -> void {
	static_cast<Window*>(glfwGetWindowUserPointer(window))->
		SetFrameBufferResized(true);
}

auto Window::GetFrameBufferSize() const -> std::pair<int, int> {
	auto width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	return std::make_pair(width, height);
}

auto Window::GetRequiredExtensions() -> std::pair<const char**, uint32_t> {
	uint32_t glfwExtensionCount = 0;
	auto     glfwExtensions     = glfwGetRequiredInstanceExtensions(
		&glfwExtensionCount
	);
	return std::make_pair(glfwExtensions, glfwExtensionCount);
}

auto Window::CreateWindow() -> void {
	CreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
}

auto Window::CreateWindow(const int width, const int height) -> void {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, FrameBufferResizeCallback);
}

auto Window::CreateSurface(const VkInstance& instance) -> void {
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

auto Window::DestroyWindow() const -> void {
	glfwDestroyWindow(window);
	glfwTerminate();
}

auto Window::OnRecreateSwapChain() const -> void {
	auto width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}
}
