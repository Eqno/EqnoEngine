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

class Draw;
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

  VkImage colorImage;
  VkDeviceMemory colorImageMemory;
  VkImageView colorImageView;

  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;

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
  void CreateColorResource(const Device& device);
  void CreateDepthResources(const Device& device);
  void TransitionDepthImageLayout(const Device& device);
  void CreateFrameBuffers(const VkDevice& device,
                          const VkRenderPass& colorRenderPass,
                          const VkRenderPass& zPrePassRenderPass,
                          const VkRenderPass& shadowMapRenderPass);

  VkSurfaceFormatKHR ChooseSurfaceFormat(
      const SurfaceFormats& availableFormats) const;

  static VkPresentModeKHR ChoosePresentMode(
      const PresentModes& availablePresentModes);

  static VkExtent2D ChooseSwapExtent(
      const VkSurfaceCapabilitiesKHR& capabilities, const Window& window);

  const VkSwapchainKHR& Get() const { return chain; }
  const VkExtent2D& GetExtent() const { return extent; }

  VkFormat GetImageFormat() const { return imageFormat; }
  VkFormat GetZPrePassDepthFormat() const {
    return zPrePassDepth.GetDepthFormat();
  }
  VkFormat GetShadowMapDepthFormat() const {
    return shadowMapDepths[0].GetDepthFormat();
  }

  uint32_t GetExtentWidth() const { return extent.width; }
  uint32_t GetExtentHeight() const { return extent.height; }

  uint32_t GetShadowMapWidth() const;
  uint32_t GetShadowMapHeight() const;

  VkFramebuffer GetColorFrameBufferByIndex(uint32_t index) const {
    return colorFrameBuffers[index];
  }
  VkFramebuffer GetZPrePassFrameBuffer() const { return zPrePassFrameBuffer; }
  VkFramebuffer GetShadowMapFrameBufferByIndex(uint32_t index) const {
    return shadowMapFrameBuffers[index];
  }

  size_t GetShadowMapDepthNum() { return shadowMapDepths.size(); }
  Depth& GetShadowMapDepthByIndex(uint32_t index) {
    return shadowMapDepths[index];
  }
  VkImage GetShadowMapDepthImageByIndex(uint32_t index) {
    return shadowMapDepths[index].GetDepthImage();
  }
  VkSampler GetShadowMapDepthSamplerByIndex(uint32_t index) {
    return shadowMapDepths[index].GetDepthSampler();
  }
  VkImageView GetShadowMapDepthImageViewByIndex(uint32_t index) {
    return shadowMapDepths[index].GetDepthImageView();
  }

  void CreateSwapChain(const Device& device, const Window& window);
  void CreateImageViews(const VkDevice& device);
  void CreateColorFrameBuffers(const VkDevice& device,
                               const VkRenderPass& colorRenderPass);
  void CreateZPrePassFrameBuffer(const VkDevice& device,
                                 const VkRenderPass& zPrePassRenderPass);
  void CreateShadowMapFrameBuffers(const VkDevice& device,
                                   const VkRenderPass& shadowMapRenderPass,
                                   const uint32_t shadowMapWidth,
                                   const uint32_t shadowMapHeight);

  void RecreateSwapChain(const Device& device, const Window& window,
                         std::unordered_map<std::string, Draw*>& draws,
                         const VkRenderPass& colorRenderPass,
                         const VkRenderPass& zPrePassRenderPass,
                         const VkRenderPass& shadowMapRenderPass);

  void CreateRenderTarget(const Device& device, const Window& window);
  void CreateRenderTarget(const std::string& format, const std::string& space,
                          const Device& device, const Window& window);
  void CleanupRenderTarget(const VkDevice& device) const;
  void DestroyColorResource(const VkDevice& device) const;
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
