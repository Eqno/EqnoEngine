#pragma once

#include "pipeline.h"
#include "swapchain.h"
#include "window.h"

class Mesh;

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

	void CreateCommandBuffers(const VkDevice& device, int maxFramesInFlight);
	void RecordCommandBuffer(const Mesh& mesh,
		const Pipeline& pipeline,
		const SwapChain& swapChain,
		uint32_t imageIndex) const;
	void CopyCommandBuffer(const Device& device,
		const VkBuffer& srcBuffer,
		const VkBuffer& dstBuffer,
		const VkDeviceSize& size) const;
	[[nodiscard]] VkCommandBuffer BeginSingleTimeCommands(
		const VkDevice& device) const;
	void EndSingleTimeCommands(const Device& device,
		VkCommandBuffer commandBuffer) const;
	void DrawFrame(const Mesh& mesh,
		const Device& device,
		Depth& depth,
		Window& window,
		const Pipeline& pipeline,
		SwapChain& swapChain);
	void CreateSyncObjects(const VkDevice& device, int maxFramesInFlight);
	void DestroySyncObjects(const VkDevice& device,
		int maxFramesInFlight) const;

	[[nodiscard]] const VkCommandPool& GetCommandPool() const {
		return commandPool;
	}

	[[nodiscard]] const std::vector<VkCommandBuffer>&
	GetCommandBuffers() const {
		return commandBuffers;
	}
};
