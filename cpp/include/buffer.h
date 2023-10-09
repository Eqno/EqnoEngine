#pragma once

#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

#include "device.h"

class Buffer {
public:
	static uint32_t MemoryType(
		const VkPhysicalDevice& physicalDevice,
		const glm::uint32_t& typeFilter,
		const VkMemoryPropertyFlags& properties
	);

	static void CreateBuffer(
		const Device& device,
		const VkDeviceSize& size,
		const VkBufferUsageFlags& usage,
		const VkMemoryPropertyFlags& properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory
	);

	static void CopyBuffer(
		const Device& device,
		const VkBuffer& srcBuffer,
		const VkBuffer& dstBuffer,
		const VkDeviceSize& size,
		const VkCommandPool& commandPool
	);
};
