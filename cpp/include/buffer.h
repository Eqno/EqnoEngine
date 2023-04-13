#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

#include "device.h"

class Buffer {
public:
	static auto FindMemoryType(
		const VkPhysicalDevice&      physicalDevice,
		const glm::uint32_t&         typeFilter,
		const VkMemoryPropertyFlags& properties
	) -> uint32_t;

	static auto CreateBuffer(
		const Device&         device,
		VkDeviceSize          size,
		VkBufferUsageFlags    usage,
		VkMemoryPropertyFlags properties,
		VkBuffer&             buffer,
		VkDeviceMemory&       bufferMemory
	) -> void;

	static auto CopyBuffer(
		const Device&        device,
		VkBuffer             srcBuffer,
		VkBuffer             dstBuffer,
		VkDeviceSize         size,
		const VkCommandPool& commandPool
	) -> void;
};
