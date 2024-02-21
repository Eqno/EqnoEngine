#pragma once

#include <unordered_map>

#include "base.h"
#include "config.h"
#include "device.h"
#include "swapchain.h"
#include "uniform.h"
#include "window.h"

#define DEFINE_GET_RENDER_PASS_AND_COMMAND_BUFFERS(lower, upper)     \
  [[nodiscard]] const VkRenderPass& Get##upper##RenderPass() const { \
    return lower##RenderPass;                                        \
  }                                                                  \
  [[nodiscard]] const std::vector<VkCommandBuffer>&                  \
      Get##upper##CommandBuffers() const {                           \
    return lower##CommandBuffers;                                    \
  }

class Mesh;
class Draw;
class Depth;

class Render : public Base {
  SwapChain swapChain;
  uint32_t currentFrame = 0;

  std::vector<VkFence> colorInFlightFences;
  std::vector<VkFence> zPrePassInFlightFences;
  int maxFramesInFlight = VulkanConfig::MAX_FRAMES_IN_FLIGHT;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> zPrePassFinishedSemaphores;
  std::vector<VkSemaphore> shadowMapFinishedSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;

  VkRenderPass colorRenderPass;
  VkRenderPass zPrePassRenderPass;
  VkRenderPass shadowMapRenderPass;

  void CreateColorRenderPass(const Device& device);
  void CreateZPrePassRenderPass(const Device& device);
  void CreateShadowMapRenderPass(const Device& device);
  void CreateCommandBuffers(const VkDevice& device,
                            std::vector<VkCommandBuffer>& commandBuffers);

  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> colorCommandBuffers;
  std::vector<VkCommandBuffer> zPrePassCommandBuffers;
  std::vector<VkCommandBuffer> shadowMapCommandBuffers;

  void CreateRenderPasses(const Device& device);
  void CreateCommandPool(const Device& device, const VkSurfaceKHR& surface);
  void CreateSyncObjects(const VkDevice& device);
  void CreateCommandBuffersSet(const VkDevice& device);

  void DestroyRenderPasses(const VkDevice& device) const;
  void DestroyCommandPool(const VkDevice& device) const;
  void DestroySyncObjects(const VkDevice& device) const;

  void RecordZPrePassCommandBuffer(
      std::unordered_map<std::string, Draw*>& draws) const;
  void RecordShadowMapCommandBuffer(
      std::unordered_map<std::string, Draw*>& draws) const;
  void RecordColorCommandBuffer(std::unordered_map<std::string, Draw*>& draws,
                                uint32_t imageIndex) const;

  void SubmitCommandBuffer(const Device& device,
                           const VkSemaphore& waitSemaphore,
                           const VkCommandBuffer& commandBuffer,
                           const VkSemaphore& signalSemaphore,
                           const VkFence waitFence);

 public:
  virtual void TriggerRegisterMember() override { RegisterMember(swapChain); }
  void CreateRenderResources(const std::string& imageFormat,
                             const std::string& colorSpace,
                             const int shadowMapWidth,
                             const int shadowMapHeight, const Device& device,
                             const Window& window) {
    swapChain.CreateRenderTarget(imageFormat, colorSpace, device, window);
    swapChain.SetShadowMapSize(shadowMapWidth, shadowMapHeight);
    swapChain.CreateDepthResources(device);

    CreateRenderPasses(device);
    CreateCommandPool(device, window.GetSurface());

    swapChain.TransitionDepthImageLayout(device);
    swapChain.CreateFrameBuffers(device.GetLogical(), colorRenderPass,
                                 zPrePassRenderPass, shadowMapRenderPass);

    CreateCommandBuffersSet(device.GetLogical());
    CreateSyncObjects(device.GetLogical());
  }

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

  [[nodiscard]] const VkCommandPool& GetCommandPool() const {
    return commandPool;
  }

  DEFINE_GET_RENDER_PASS_AND_COMMAND_BUFFERS(color, Color)
  DEFINE_GET_RENDER_PASS_AND_COMMAND_BUFFERS(zPrePass, ZPrePass)
  DEFINE_GET_RENDER_PASS_AND_COMMAND_BUFFERS(shadowMap, ShadowMap)

  [[nodiscard]] int GetCurrentFrame() const { return currentFrame; }
  [[nodiscard]] int GetMaxFramesInFlight() const { return maxFramesInFlight; }
  [[nodiscard]] float GetViewportAspect() {
    return swapChain.GetViewportAspect();
  }
};

#undef DEFINE_GET_RENDER_PASS_AND_COMMAND_BUFFERS