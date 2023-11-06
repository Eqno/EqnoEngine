#include "../include/buffer.h"

#include <stdexcept>

#include "../include/pipeline.h"
#include "../include/render.h"
#include "../include/uniform.h"
#include "../include/device.h"

uint32_t Buffer::MemoryType(const VkPhysicalDevice& physicalDevice,
	const glm::uint32_t& typeFilter,
	const VkMemoryPropertyFlags& properties) {
	VkPhysicalDeviceMemoryProperties memProperties {};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].
			propertyFlags & properties) == properties) {
			return i;
		}
	}
	throw std::runtime_error("Failed to find suitable memory type!");
}

void Buffer::CreateBuffer(const Device& device,
	const VkDeviceSize& size,
	const VkBufferUsageFlags& usage,
	const VkMemoryPropertyFlags& properties,
	VkBuffer& buffer,
	VkDeviceMemory& bufferMemory) {
	const VkBufferCreateInfo bufferInfo {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	if (vkCreateBuffer(device.GetLogical(), &bufferInfo, nullptr, &buffer) !=
		VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer!");
	}

	VkMemoryRequirements memRequirements {};
	vkGetBufferMemoryRequirements(device.GetLogical(), buffer,
		&memRequirements);

	const VkMemoryAllocateInfo allocInfo {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = MemoryType(device.GetPhysical(),
			memRequirements.memoryTypeBits, properties),
	};
	if (vkAllocateMemory(device.GetLogical(), &allocInfo, nullptr,
		&bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory!");
	}
	vkBindBufferMemory(device.GetLogical(), buffer, bufferMemory, 0);
}

void Buffer::CreateVertexBuffer(const Device& device,
	const std::vector<Vertex>& vertices,
	const Render& render) {
	const auto bufferSize = vertices.empty()
	                        ? 0
	                        : sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
		stagingBufferMemory);

	void* vertexData;
	vkMapMemory(device.GetLogical(), stagingBufferMemory, 0, bufferSize, 0,
		&vertexData);
	memcpy(vertexData, vertices.data(), bufferSize);
	vkUnmapMemory(device.GetLogical(), stagingBufferMemory);

	CreateBuffer(device, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
	render.CopyCommandBuffer(device, stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(device.GetLogical(), stagingBuffer, nullptr);
	vkFreeMemory(device.GetLogical(), stagingBufferMemory, nullptr);
}

void Buffer::CreateIndexBuffer(const Device& device,
	const std::vector<uint32_t>& indices,
	const Render& render) {
	const auto bufferSize = indices.empty()
	                        ? 0
	                        : sizeof(indices[0]) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
		stagingBufferMemory);

	void* indexData;
	vkMapMemory(device.GetLogical(), stagingBufferMemory, 0, bufferSize, 0,
		&indexData);
	memcpy(indexData, indices.data(), bufferSize);
	vkUnmapMemory(device.GetLogical(), stagingBufferMemory);

	CreateBuffer(device, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

	render.CopyCommandBuffer(device, stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(device.GetLogical(), stagingBuffer, nullptr);
	vkFreeMemory(device.GetLogical(), stagingBufferMemory, nullptr);
}

void Buffer::Create(const Device& device,
	const std::vector<Vertex>& vertices,
	const std::vector<uint32_t>& indices,
	const Render& render) {
	CreateVertexBuffer(device, vertices, render);
	CreateIndexBuffer(device, indices, render);
}

void Buffer::Destroy(const VkDevice& device) const {
	vkDestroyBuffer(device, indexBuffer, nullptr);
	vkFreeMemory(device, indexBufferMemory, nullptr);

	vkDestroyBuffer(device, vertexBuffer, nullptr);
	vkFreeMemory(device, vertexBufferMemory, nullptr);
}
