#pragma once

#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

#include "device.h"
#include "pipeline.h"
#include "swapchain.h"
#include "uniform.h"

class Buffer {
	const std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0 };

	VkBuffer vertexBuffer {};
	VkDeviceMemory vertexBufferMemory {};

	VkBuffer indexBuffer {};
	VkDeviceMemory indexBufferMemory {};

	VkCommandPool commandPool {};
	std::vector<VkCommandBuffer> commandBuffers;

	static uint32_t MemoryType(
		const VkPhysicalDevice& physicalDevice,
		const glm::uint32_t& typeFilter,
		const VkMemoryPropertyFlags& properties
	);

	static void CopyBuffer(
		const Device& device,
		const VkBuffer& srcBuffer,
		const VkBuffer& dstBuffer,
		const VkDeviceSize& size,
		const VkCommandPool& commandPool
	);

public:
	Buffer() = default;

	static void CreateBuffer(
		const Device& device,
		const VkDeviceSize& size,
		const VkBufferUsageFlags& usage,
		const VkMemoryPropertyFlags& properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory
	);

	void CreateVertexBuffer(const Device& device);
	void CreateIndexBuffer(const Device& device);
	void CreateCommandBuffers(const Device& device);
	void RecordCommandBuffer(
		const Pipeline& pipeline,
		const SwapChain& swapChain,
		const Descriptor& descriptor,
		uint32_t currentFrame,
		VkCommandBuffer commandBuffer,
		uint32_t imageIndex
	) const;

	[[nodiscard]] const VkBuffer& GetVertexBuffer() const { return vertexBuffer; }
	[[nodiscard]] const VkDeviceMemory& GetVertexBufferMemory() const { return vertexBufferMemory; }

	[[nodiscard]] const VkBuffer& GetIndexBuffer() const { return indexBuffer; }
	[[nodiscard]] const VkDeviceMemory& GetIndexBufferMemory() const { return indexBufferMemory; }

	[[nodiscard]] VkCommandPool& GetCommandPool() { return commandPool; }
	[[nodiscard]] const std::vector<VkCommandBuffer>& GetCommandBuffers() const { return commandBuffers; }

	[[nodiscard]] const VkCommandBuffer& GetCommandBufferByFrame(const uint32_t frame) const {
		return commandBuffers[frame];
	}
};
