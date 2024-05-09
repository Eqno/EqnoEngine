#include "../include/window.h"

#include <Engine/RHI/Vulkan/include/vulkan.h>
#include <Engine/System/include/BaseInput.h>
#include <Engine/Utility/include/TypeUtils.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <stdexcept>

#include "../include/config.h"
using namespace Input;

void FrameBufferResizeCallback([[maybe_unused]] GLFWwindow* window,
                               [[maybe_unused]] int width,
                               [[maybe_unused]] int height) {
  static_cast<VkWindow*>(glfwGetWindowUserPointer(window))
      ->SetFrameBufferResized(true);
}

std::pair<int, int> VkWindow::GetFrameBufferSize() const {
  auto width = 0, height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  return std::make_pair(width, height);
}

std::pair<const char**, uint32_t> VkWindow::GetRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  return std::make_pair(glfwExtensions, glfwExtensionCount);
}

void VkWindow::CreateVkWindow(const int width, const int height,
                              const std::string& title) {
  if (static_cast<Vulkan*>(owner)->GetEnableEditor() == false) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  }

  window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, FrameBufferResizeCallback);

  if (static_cast<Vulkan*>(owner)->GetEnableEditor() &&
      static_cast<Vulkan*>(owner)->GetLaunchSceneInEditor()) {
    HWND graphicsWindowHWND = glfwGetWin32Window(window);
    HWND parentWindowHWND =
        glfwGetWin32Window(static_cast<Vulkan*>(owner)->GetParentWindow());

    SetWindowLong(graphicsWindowHWND, GWL_STYLE, WS_VISIBLE);
    MoveWindow(graphicsWindowHWND, 0, 0, width, height, TRUE);
    SetParent(graphicsWindowHWND, parentWindowHWND);
  }

  glfwSetKeyCallback(window, Key::ButtonCallback);
  glfwSetScrollCallback(window, Mouse::ScrollCallback);
  glfwSetCursorPosCallback(window, Mouse::PositionCallback);
  glfwSetMouseButtonCallback(window, Mouse::ButtonCallback);
}

void VkWindow::CreateSurface(const VkInstance& instance) {
  if (glfwCreateWindowSurface(instance, window, nullptr, &surface) !=
      VK_SUCCESS) {
    PRINT_AND_THROW_ERROR("Failed to create window surface!");
  }
}

void VkWindow::DestroySurface(const VkInstance& instance) const {
  vkDestroySurfaceKHR(instance, surface, nullptr);
}

void VkWindow::DestroyWindow() const {
  if (static_cast<Vulkan*>(owner)->GetEnableEditor() == false ||
      static_cast<Vulkan*>(owner)->GetLaunchSceneInEditor() == false) {
    glfwDestroyWindow(window);
  }
  if (static_cast<Vulkan*>(owner)->GetEnableEditor() == false) {
    glfwTerminate();
  }
}

void VkWindow::OnRecreateSwapChain() const {
  auto width = 0, height = 0;
  glfwGetFramebufferSize(window, &width, &height);

  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(window, &width, &height);
    glfwWaitEvents();
  }
}
