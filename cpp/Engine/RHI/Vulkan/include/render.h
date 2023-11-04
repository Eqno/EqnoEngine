#pragma once

#include "config.h"
#include "pipeline.h"
#include "swapchain.h"
#include "window.h"

class Mesh;

class Render {
	uint32_t currentFrame = 0;
	std::vector<VkFence> inFlightFences;
	int maxFramesInFlight = Config::MAX_FRAMES_IN_FLIGHT;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;

	VkCommandPool commandPool {};
	std::vector<VkCommandBuffer> commandBuffers;

public:
	void CreateCommandPool(const Device& device, const VkSurfaceKHR& surface);
	void DestroyCommandPool(const VkDevice& device) const;

	void CreateCommandBuffers(const VkDevice& device);
	void RecordCommandBuffer(const std::vector<Mesh>& meshes,
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
	void DrawFrame(const std::vector<Mesh>& meshes,
		const Device& device,
		Depth& depth,
		Window& window,
		const Pipeline& pipeline,
		SwapChain& swapChain);
	void CreateSyncObjects(const VkDevice& device);
	void DestroySyncObjects(const VkDevice& device) const;

	[[nodiscard]] const VkCommandPool& GetCommandPool() const {
		return commandPool;
	}

	[[nodiscard]] const std::vector<VkCommandBuffer>&
	GetCommandBuffers() const {
		return commandBuffers;
	}

	[[nodiscard]] int GetMaxFramesInFlight() const {
		return maxFramesInFlight;
	}
};
