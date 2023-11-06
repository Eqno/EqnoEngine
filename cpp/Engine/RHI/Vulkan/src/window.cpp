#include "../include/window.h"

#include <stdexcept>

#include "../include/config.h"

void FrameBufferResizeCallback([[maybe_unused]] GLFWwindow* window,
	[[maybe_unused]] int width,
	[[maybe_unused]] int height) {
	static_cast<Window*>(glfwGetWindowUserPointer(window))->
		SetFrameBufferResized(true);
}

std::pair<int, int> Window::GetFrameBufferSize() const {
	auto width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	return std::make_pair(width, height);
}

std::pair<const char**, uint32_t> Window::GetRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	auto glfwExtensions =
		glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	return std::make_pair(glfwExtensions, glfwExtensionCount);
}

void Window::CreateWindow() {
	CreateWindow(VulkanConfig::DEFAULT_WINDOW_WIDTH,
		VulkanConfig::DEFAULT_WINDOW_HEIGHT);
}

void Window::CreateWindow(const int width, const int height) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, FrameBufferResizeCallback);
}

void Window::CreateSurface(const VkInstance& instance) {
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) !=
	    VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface!");
	}
}

void Window::DestroySurface(const VkInstance& instance) const {
	vkDestroySurfaceKHR(instance, surface, nullptr);
}

void Window::DestroyWindow() const {
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Window::OnRecreateSwapChain() const {
	auto width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);

	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}
}
