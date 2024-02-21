#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

#include "base.h"
#include "depth.h"
#include "utils.h"

using PresentModes = std::vector<VkPresentModeKHR>;
using SurfaceFormats = std::vector<VkSurfaceFormatKHR>;

class Window;
class Device;
class Render;

class SwapChain : public Base {
  Depth zPrePassDepth;
  Depth shadowMapDepth;

  VkSwapchainKHR chain;
  VkExtent2D extent;
  VkFormat imageFormat;

  VkFormat surfaceFormat = VK_FORMAT_R8G8B8A8_SRGB;
  VkColorSpaceKHR surfaceColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

  int shadowMapWidth = 0;
  int shadowMapHeight = 0;

  std::vector<VkImage> colorImages;
  std::vector<VkImageView> colorImageViews;

  std::vector<VkFramebuffer> colorFrameBuffers;
  VkFramebuffer zPrePassFrameBuffer;
  VkFramebuffer shadowMapFrameBuffer;

 public:
  virtual void TriggerRegisterMember() override {
    RegisterMember(zPrePassDepth, shadowMapDepth);
  }
  void SetShadowMapSize(const int width, const int Height) {
    shadowMapWidth = width;
    shadowMapHeight = Height;
  }
  void CreateDepthResources(const Device& device);
  void TransitionDepthImageLayout(const Device& device);
  void CreateFrameBuffers(const VkDevice& device,
                          const VkRenderPass& colorRenderPass,
                          const VkRenderPass& zPrePassRenderPass,
                          const VkRenderPass& shadowMapRenderPass) {
    CreateColorFrameBuffers(device, colorRenderPass);
    CreateZPrePassFrameBuffers(device, zPrePassRenderPass);
    CreateShadowMapFrameBuffers(device, shadowMapRenderPass,
                                GetShadowMapWidth(), GetShadowMapHeight());
  }

  [[nodiscard]] VkSurfaceFormatKHR ChooseSurfaceFormat(
      const SurfaceFormats& availableFormats) const;

  static VkPresentModeKHR ChoosePresentMode(
      const PresentModes& availablePresentModes);

  [[nodiscard]] static VkExtent2D ChooseSwapExtent(
      const VkSurfaceCapabilitiesKHR& capabilities, const Window& window);

  [[nodiscard]] const VkSwapchainKHR& Get() const { return chain; }

  [[nodiscard]] const VkExtent2D& GetExtent() const { return extent; }

  [[nodiscard]] const VkFormat& GetImageFormat() const { return imageFormat; }

  [[nodiscard]] const VkFramebuffer& GetColorFrameBufferByIndex(
      uint32_t index) const {
    return colorFrameBuffers[index];
  }
  [[nodiscard]] const VkFramebuffer& GetZPrePassFrameBuffer() const {
    return zPrePassFrameBuffer;
  }
  [[nodiscard]] const VkFramebuffer& GetShadowMapFrameBuffer() const {
    return shadowMapFrameBuffer;
  }

  [[nodiscard]] uint32_t GetShadowMapWidth() const {
    return shadowMapWidth >= 0 ? shadowMapWidth : extent.width;
  }
  [[nodiscard]] uint32_t GetShadowMapHeight() const {
    return shadowMapHeight >= 0 ? shadowMapHeight : extent.height;
  }

  VkFormat GetZPrePassDepthFormat() { return zPrePassDepth.GetDepthFormat(); }
  VkFormat GetShadowMapDepthFormat() { return shadowMapDepth.GetDepthFormat(); }

  void CreateSwapChain(const Device& device, const Window& window);
  void CreateColorImageViews(const VkDevice& device);
  void CreateColorFrameBuffers(const VkDevice& device,
                               const VkRenderPass& colorRenderPass);
  void CreateZPrePassFrameBuffers(const VkDevice& device,
                                  const VkRenderPass& zPrePassRenderPass);
  void CreateShadowMapFrameBuffers(const VkDevice& device,
                                   const VkRenderPass& shadowMapRenderPass,
                                   const uint32_t shadowMapWidth,
                                   const uint32_t shadowMapHeight);

  void RecreateSwapChain(const Device& device, const Window& window,
                         const VkRenderPass& colorRenderPass,
                         const VkRenderPass& zPrePassRenderPass,
                         const VkRenderPass& shadowMapRenderPass);

  void CreateRenderTarget(const Device& device, const Window& window);
  void CreateRenderTarget(const std::string& format, const std::string& space,
                          const Device& device, const Window& window);
  void CleanupRenderTarget(const VkDevice& device) const;
  void DestroyDepthResource(const VkDevice& device) {
    zPrePassDepth.DestroyDepthResource(device);
    shadowMapDepth.DestroyDepthResource(device);
  }

  float GetViewportAspect() {
    return static_cast<float>(extent.width) / static_cast<float>(extent.height);
  }
};
