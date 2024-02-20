#pragma once

#include <unordered_map>

#include "base.h"
#include "config.h"
#include "device.h"
#include "swapchain.h"
#include "uniform.h"
#include "window.h"

class Mesh;
class Draw;
class Depth;

class Render : public Base {
  SwapChain swapChain;
  uint32_t currentFrame = 0;
  std::vector<VkFence> inFlightFences;
  int maxFramesInFlight = VulkanConfig::MAX_FRAMES_IN_FLIGHT;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;

  VkRenderPass colorRenderPass;
  VkRenderPass zPrePassRenderPass;
  VkRenderPass shadowMapRenderPass;

  void CreateColorRenderPass(const Device& device);
  void CreateZPrePassRenderPass(const Device& device);
  void CreateShadowMapRenderPass(const Device& device);

  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;

  void CreateRenderPasses(const Device& device);
  void CreateCommandPool(const Device& device, const VkSurfaceKHR& surface);
  void CreateSyncObjects(const VkDevice& device);
  void CreateCommandBuffers(const VkDevice& device);

  void DestroyRenderPasses(const VkDevice& device) const;
  void DestroyCommandPool(const VkDevice& device) const;
  void DestroySyncObjects(const VkDevice& device) const;

 public:
  virtual void TriggerRegisterMember() override { RegisterMember(swapChain); }
  void CreateRenderResources(const std::string& imageFormat,
                             const std::string& colorSpace,
                             const int shadowMapWidth,
                             const int shadowMapHeight, const Device& device,
                             const Window& window) {
    swapChain.CreateRenderTarget(imageFormat, colorSpace, device, window);
    swapChain.SetShadowMapSize(shadowMapWidth, shadowMapHeight);

    CreateRenderPasses(device);
    CreateCommandPool(device, window.GetSurface());

    swapChain.CreateDepthResources(device);
    swapChain.CreateFrameBuffers(device.GetLogical(), colorRenderPass,
                                 zPrePassRenderPass, shadowMapRenderPass);

    CreateCommandBuffers(device.GetLogical());
    CreateSyncObjects(device.GetLogical());
  }

  void RecordCommandBuffer(std::unordered_map<std::string, Draw*>& draws,
                           const SwapChain& swapChain,
                           uint32_t imageIndex) const;
  void CopyCommandBuffer(const Device& device, const VkBuffer& srcBuffer,
                         const VkBuffer& dstBuffer,
                         const VkDeviceSize& size) const;

  void BeginSingleTimeCommands(const VkDevice& device,
                               VkCommandBuffer* commandBuffer) const;
  void EndSingleTimeCommands(const Device& device,
                             VkCommandBuffer* commandBuffer) const;

  void DrawFrame(const Device& device,
                 std::unordered_map<std::string, Draw*>& draws, Window& window);

  void DestroyRenderResources(const VkDevice& device) {
    swapChain.DestroyDepthResource(device);
    swapChain.CleanupRenderTarget(device);
    DestroyRenderPasses(device);
    DestroySyncObjects(device);
    DestroyCommandPool(device);
  }

  [[nodiscard]] const VkFormat& GetSwapChainImageFormat() const {
    return swapChain.GetImageFormat();
  }

  [[nodiscard]] const VkRenderPass& GetRenderPass() const {
    return colorRenderPass;
  }

  [[nodiscard]] const VkCommandPool& GetCommandPool() const {
    return commandPool;
  }

  [[nodiscard]] const std::vector<VkCommandBuffer>& GetCommandBuffers() const {
    return commandBuffers;
  }

  [[nodiscard]] int GetCurrentFrame() const { return currentFrame; }
  [[nodiscard]] int GetMaxFramesInFlight() const { return maxFramesInFlight; }
  [[nodiscard]] float GetViewportAspect() {
    return swapChain.GetViewportAspect();
  }
};
