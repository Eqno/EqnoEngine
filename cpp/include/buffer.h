#pragma once

#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

#include "device.h"
#include "render.h"

class Buffer {
	VkBuffer vertexBuffer {};
	VkDeviceMemory vertexBufferMemory {};

	VkBuffer indexBuffer {};
	VkDeviceMemory indexBufferMemory {};

	static uint32_t MemoryType(
		const VkPhysicalDevice& physicalDevice,
		const glm::uint32_t& typeFilter,
		const VkMemoryPropertyFlags& properties
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

	void CreateVertexBuffer(const Device& device, const Render& render);
	void CreateIndexBuffer(const Device& device, const Render& render);
	void CleanupBuffers(const VkDevice& device) const;

	[[nodiscard]] const VkBuffer& GetVertexBuffer() const { return vertexBuffer; }
	[[nodiscard]] const VkDeviceMemory& GetVertexBufferMemory() const { return vertexBufferMemory; }

	[[nodiscard]] const VkBuffer& GetIndexBuffer() const { return indexBuffer; }
	[[nodiscard]] const VkDeviceMemory& GetIndexBufferMemory() const { return indexBufferMemory; }
};
