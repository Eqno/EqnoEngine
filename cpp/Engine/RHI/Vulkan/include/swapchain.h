#pragma once

#include <Engine/Utility/include/TypeUtils.h>
#include <vulkan/vulkan_core.h>

#include <array>
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
  std::array<Depth, MaxLightNum> shadowMapDepths;

  VkSwapchainKHR chain;
  VkExtent2D extent;
  VkFormat imageFormat;

  VkFormat surfaceFormat = VK_FORMAT_R8G8B8A8_SRGB;
  VkColorSpaceKHR surfaceColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

  int shadowMapWidth = 0;
  int shadowMapHeight = 0;

  std::vector<VkImage> colorImages;
  std::vector<VkImageView> colorImageViews;

  VkFramebuffer zPrePassFrameBuffer;
  std::vector<VkFramebuffer> colorFrameBuffers;
  std::vector<VkFramebuffer> shadowMapFrameBuffers;

 public:
  virtual void TriggerRegisterMember() override {
    RegisterMember(zPrePassDepth);
    for (Depth& depth : shadowMapDepths) {
      RegisterMember(depth);
    }
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
    CreateZPrePassFrameBuffer(device, zPrePassRenderPass);
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
  [[nodiscard]] const VkFramebuffer& GetShadowMapFrameBufferByIndex(
      uint32_t index) const {
    return shadowMapFrameBuffers[index];
  }

  [[nodiscard]] uint32_t GetShadowMapWidth() const {
    return shadowMapWidth >= 0 ? shadowMapWidth : extent.width;
  }
  [[nodiscard]] uint32_t GetShadowMapHeight() const {
    return shadowMapHeight >= 0 ? shadowMapHeight : extent.height;
  }

  [[nodiscard]] size_t GetShadowMapDepthNum() { return shadowMapDepths.size(); }
  [[nodiscard]] const VkImage& GetShadowMapDepthImageByIndex(uint32_t index) {
    return shadowMapDepths[index].GetDepthImage();
  }
  [[nodiscard]] const VkSampler& GetShadowMapDepthSamplerByIndex(
      uint32_t index) {
    return shadowMapDepths[index].GetDepthSampler();
  }
  [[nodiscard]] const VkImageView& GetShadowMapDepthImageViewByIndex(
      uint32_t index) {
    return shadowMapDepths[index].GetDepthImageView();
  }

  [[nodiscard]] VkFormat GetZPrePassDepthFormat() const {
    return zPrePassDepth.GetDepthFormat();
  }
  [[nodiscard]] VkFormat GetShadowMapDepthFormat() const {
    return shadowMapDepths[0].GetDepthFormat();
  }

  void CreateSwapChain(const Device& device, const Window& window);
  void CreateColorImageViews(const VkDevice& device);
  void CreateColorFrameBuffers(const VkDevice& device,
                               const VkRenderPass& colorRenderPass);
  void CreateZPrePassFrameBuffer(const VkDevice& device,
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
    for (Depth& depth : shadowMapDepths) {
      depth.DestroyDepthResource(device);
    }
  }

  float GetViewportAspect() {
    return static_cast<float>(extent.width) / static_cast<float>(extent.height);
  }
};
