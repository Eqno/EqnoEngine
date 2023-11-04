#pragma once

#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

#include "device.h"
#include "data.h"
#include "render.h"

class Buffer {
	VkBuffer vertexBuffer {};
	VkDeviceMemory vertexBufferMemory {};

	VkBuffer indexBuffer {};
	VkDeviceMemory indexBufferMemory {};

	void CreateVertexBuffer(const Device& device,
		const std::vector<Vertex>& vertices,
		const Render& render);
	void CreateIndexBuffer(const Device& device,
		const std::vector<uint32_t>& indices,
		const Render& render);

public:
	Buffer() = default;

	static uint32_t MemoryType(const VkPhysicalDevice& physicalDevice,
		const glm::uint32_t& typeFilter,
		const VkMemoryPropertyFlags& properties);

	static void CreateBuffer(const Device& device,
		const VkDeviceSize& size,
		const VkBufferUsageFlags& usage,
		const VkMemoryPropertyFlags& properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory);

	void Create(const Device& device,
		const std::vector<Vertex>& vertices,
		const std::vector<uint32_t>& indices,
		const Render& render);
	void Destroy(const VkDevice& device) const;

	[[nodiscard]] const VkBuffer& GetVertexBuffer() const {
		return vertexBuffer;
	}

	[[nodiscard]] const VkDeviceMemory& GetVertexBufferMemory() const {
		return vertexBufferMemory;
	}

	[[nodiscard]] const VkBuffer& GetIndexBuffer() const {
		return indexBuffer;
	}

	[[nodiscard]] const VkDeviceMemory& GetIndexBufferMemory() const {
		return indexBufferMemory;
	}
};
