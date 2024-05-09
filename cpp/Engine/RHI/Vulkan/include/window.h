#pragma once
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include <utility>
#include <xstring>

#include "base.h"

struct GLFWwindow;

class VkWindow : public Base {
  VkSurfaceKHR surface;
  GLFWwindow* window = nullptr;
  bool frameBufferResized = false;

 public:
  GLFWwindow* GetWindow() const { return window; }
  const VkSurfaceKHR& GetSurface() const { return surface; }

  std::pair<int, int> GetFrameBufferSize() const;
  const bool& GetFrameBufferResized() const { return frameBufferResized; }
  static std::pair<const char**, uint32_t> GetRequiredExtensions();

  void SetFrameBufferResized(const bool resized) {
    frameBufferResized = resized;
  }

  void CreateVkWindow(int width, int height, const std::string& title);

  void CreateSurface(const VkInstance& instance);
  void DestroySurface(const VkInstance& instance) const;
  void DestroyWindow() const;

  void OnRecreateSwapChain() const;
};
