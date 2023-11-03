#pragma once

#include "pipeline.h"
#include "swapchain.h"
#include "uniform.h"
#include "window.h"

class Render {
	uint32_t currentFrame = 0;
	std::vector<VkFence> inFlightFences;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;

	VkCommandPool commandPool {};
	std::vector<VkCommandBuffer> commandBuffers;

public:
	void CreateCommandPool(const Device& device, const VkSurfaceKHR& surface);
	void DestroyCommandPool(const VkDevice& device) const;

	void CreateCommandBuffers(const VkDevice& device);
	void RecordCommandBuffer(
		const VkBuffer& indexBuffer,
		const VkBuffer& vertexBuffer,
		const Pipeline& pipeline,
		const SwapChain& swapChain,
		const Descriptor& descriptor, const uint32_t imageIndex
	) const;
	void CopyCommandBuffer(
		const Device& device,
		const VkBuffer& srcBuffer,
		const VkBuffer& dstBuffer,
		const VkDeviceSize& size
	) const;
	[[nodiscard]] VkCommandBuffer BeginSingleTimeCommands(const VkDevice& device) const;
	void EndSingleTimeCommands(const Device& device, VkCommandBuffer commandBuffer) const;
	void DrawFrame(
		const VkBuffer& indexBuffer,
		const VkBuffer& vertexBuffer,
		const Device& device,
		Window& window,
		const Pipeline& pipeline,
		SwapChain& swapChain,
		const Descriptor& descriptor
	);
	void CreateSyncObjects(const VkDevice& device);
	void DestroySyncObjects(const VkDevice& device) const;

	[[nodiscard]] const VkCommandPool& GetCommandPool() const {
		return commandPool;
	}

	[[nodiscard]] const std::vector<VkCommandBuffer>& GetCommandBuffers() const {
		return commandBuffers;
	}
};
