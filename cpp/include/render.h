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

	const std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0 };

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

	[[nodiscard]] const std::vector<uint16_t>& GetIndices() const { return indices; }

	[[nodiscard]] const VkCommandPool& GetCommandPool() const { return commandPool; }
	[[nodiscard]] const std::vector<VkCommandBuffer>& GetCommandBuffers() const { return commandBuffers; }

	// [[nodiscard]] const VkCommandBuffer& GetCommandBufferByFrame(const uint32_t frame) const {
	// 	return commandBuffers[frame];
	// }
};
