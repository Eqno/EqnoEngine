#pragma once

#include <unordered_map>

#include "base.h"
#include "config.h"
#include "uniform.h"
#include "window.h"

class Mesh;
class Draw;
class Depth;
class Device;
class SwapChain;

class Render : public Base {
  uint32_t currentFrame = 0;
  std::vector<VkFence> inFlightFences;
  int maxFramesInFlight = VulkanConfig::MAX_FRAMES_IN_FLIGHT;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;

  VkRenderPass renderPass{};
  VkCommandPool commandPool{};
  std::vector<VkCommandBuffer> commandBuffers;

  void DestroyRenderPass(const VkDevice& device) const;
  void DestroyCommandPool(const VkDevice& device) const;
  void DestroySyncObjects(const VkDevice& device) const;

 public:
  void CreateRenderPass(const VkFormat& imageFormat, const Device& device);
  void CreateCommandPool(const Device& device, const VkSurfaceKHR& surface);
  void CreateSyncObjects(const VkDevice& device);

  void CreateCommandBuffers(const VkDevice& device);
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
                 std::unordered_map<std::string, Draw*>& draws, Depth& depth,
                 Window& window, SwapChain& swapChain);

  void DestroyRenderResources(const VkDevice& device) const;

  [[nodiscard]] const VkRenderPass& GetRenderPass() const { return renderPass; }

  [[nodiscard]] const VkCommandPool& GetCommandPool() const {
    return commandPool;
  }

  [[nodiscard]] const std::vector<VkCommandBuffer>& GetCommandBuffers() const {
    return commandBuffers;
  }

  [[nodiscard]] int GetCurrentFrame() const { return currentFrame; }
  [[nodiscard]] int GetMaxFramesInFlight() const { return maxFramesInFlight; }
};
