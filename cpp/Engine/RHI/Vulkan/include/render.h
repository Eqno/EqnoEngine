#pragma once
#pragma once

#include <vulkan/vulkan_core.h>

#include <unordered_map>

#include "base.h"
#include "config.h"
#include "device.h"
#include "swapchain.h"
#include "uniform.h"
#include "window.h"

#define DEFINE_GET_RENDER_PASS(lower, upper)                         \
  [[nodiscard]] const VkRenderPass& Get##upper##RenderPass() const { \
    return lower##RenderPass;                                        \
  }
#define DEFINE_GET_COMMAND_BUFFERS(lower, upper)    \
  [[nodiscard]] const std::vector<VkCommandBuffer>& \
      Get##upper##CommandBuffers() const {          \
    return lower##CommandBuffers;                   \
  }

class Mesh;
class Draw;
class Depth;
class Vulkan;

class Render : public Base {
  SwapChain swapChain;
  uint32_t currentFrame = 0;
  int maxFramesInFlight = VulkanConfig::MAX_FRAMES_IN_FLIGHT;

  std::vector<VkFence> colorInFlightFences;
  std::vector<VkFence> zPrePassInFlightFences;
  std::vector<std::vector<VkFence>> shadowMapInFlightFences;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> zPrePassFinishedSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<std::vector<VkSemaphore>> shadowMapFinishedSemaphores;

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
  std::vector<std::vector<VkCommandBuffer>> shadowMapCommandBuffers;

  void CreateRenderPasses(const Device& device);
  void CreateCommandPool(const Device& device, const VkSurfaceKHR& surface);
  void CreateSyncObjects(const VkDevice& device);
  void CreateCommandBuffersSet(const VkDevice& device);

  void DestroyRenderPasses(const VkDevice& device) const;
  void DestroyCommandPool(const VkDevice& device) const;
  void DestroySyncObjects(const VkDevice& device);

  void ConvertShaderSourceToShadowMapDepth(
      const Device& device, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
  void ConvertShadowMapDepthToShaderSource(
      const Device& device, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

  void RecordZPrePassCommandBuffer(
      const Device& device, std::unordered_map<std::string, Draw*>& draws);
  void RecordShadowMapCommandBuffer(
      const Device& device, std::unordered_map<std::string, Draw*>& draws,
      BaseLight* light);
  void RecordColorCommandBuffer(const Device& device,
                                std::unordered_map<std::string, Draw*>& draws,
                                uint32_t imageIndex);

  void SubmitCommandBuffer(const Device& device,
                           const VkSemaphore& waitSemaphore,
                           const VkCommandBuffer& commandBuffer,
                           const VkSemaphore& signalSemaphore,
                           const VkFence waitFence);

 public:
  virtual void TriggerRegisterMember() override { RegisterMember(swapChain); }
  void CreateRenderResources(const Device& device, const Window& window,
                             const std::string& imageFormat,
                             const std::string& colorSpace) {
    swapChain.CreateRenderTarget(imageFormat, colorSpace, device, window);
    swapChain.CreateColorResource(device);
    swapChain.CreateDepthResources(device);

    CreateRenderPasses(device);
    CreateCommandPool(device, window.GetSurface());

    swapChain.TransitionDepthImageLayout(device);
    swapChain.CreateFrameBuffers(device);

    CreateCommandBuffersSet(device.GetLogical());
    CreateSyncObjects(device.GetLogical());
  }

  bool GetEnableMipmap() const;
  bool GetEnableZPrePass() const;
  bool GetEnableShadowMap() const;
  uint32_t GetShadowMapWidth() const;
  uint32_t GetShadowMapHeight() const;

  void CopyCommandBuffer(const Device& device, const VkBuffer& srcBuffer,
                         const VkBuffer& dstBuffer,
                         const VkDeviceSize& size) const;

  void BeginSingleTimeCommands(const VkDevice& device,
                               VkCommandBuffer* commandBuffer) const;
  void EndSingleTimeCommands(const Device& device,
                             VkCommandBuffer* commandBuffer) const;

  void WaitFences(
      const Device& device,
      std::unordered_map<int, std::weak_ptr<BaseLight>>& lightsById);
  void ResetFences(
      const Device& device,
      std::unordered_map<int, std::weak_ptr<BaseLight>>& lightsById);
  void DrawFrame(const Device& device,
                 std::unordered_map<std::string, Draw*>& draws,
                 std::unordered_map<int, std::weak_ptr<BaseLight>>& lightsById,
                 Window& window);

  void DestroyRenderResources(const Device& device) {
    swapChain.DestroyColorResource(device);
    swapChain.DestroyDepthResource(device.GetLogical());
    swapChain.CleanupRenderTarget(device.GetLogical());
    DestroyRenderPasses(device.GetLogical());
    DestroySyncObjects(device.GetLogical());
    DestroyCommandPool(device.GetLogical());
  }

  [[nodiscard]] const VkFormat& GetSwapChainImageFormat() const {
    return swapChain.GetImageFormat();
  }
  [[nodiscard]] const VkCommandPool& GetCommandPool() const {
    return commandPool;
  }
  [[nodiscard]] size_t GetShadowMapDepthNum() {
    return swapChain.GetShadowMapDepthNum();
  }
  [[nodiscard]] VkImage GetShadowMapDepthImageByIndex(uint32_t index) {
    return swapChain.GetShadowMapDepthImageByIndex(index);
  }
  [[nodiscard]] VkSampler GetShadowMapDepthSamplerByIndex(uint32_t index) {
    return swapChain.GetShadowMapDepthSamplerByIndex(index);
  }
  [[nodiscard]] VkImageView GetShadowMapDepthImageViewByIndex(uint32_t index) {
    return swapChain.GetShadowMapDepthImageViewByIndex(index);
  }

  [[nodiscard]] uint32_t GetSwapChainExtentWidth() const {
    return swapChain.GetExtentWidth();
  }
  [[nodiscard]] uint32_t GetSwapChainExtentHeight() const {
    return swapChain.GetExtentHeight();
  }

  DEFINE_GET_RENDER_PASS(color, Color)
  DEFINE_GET_RENDER_PASS(zPrePass, ZPrePass)
  DEFINE_GET_RENDER_PASS(shadowMap, ShadowMap)

  DEFINE_GET_COMMAND_BUFFERS(color, Color)
  DEFINE_GET_COMMAND_BUFFERS(zPrePass, ZPrePass)

  [[nodiscard]] int GetCurrentFrame() const { return currentFrame; }
  [[nodiscard]] int GetMaxFramesInFlight() const { return maxFramesInFlight; }
  [[nodiscard]] float GetViewportAspect() {
    return swapChain.GetViewportAspect();
  }
};

#undef DEFINE_GET_COMMAND_BUFFERS
#undef DEFINE_GET_RENDER_PASS