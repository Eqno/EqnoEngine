#include "../include/buffer.h"

#include <stdexcept>

#include "../include/mesh.h"
#include "../include/pipeline.h"
#include "../include/render.h"
#include "../include/uniform.h"

uint32_t Buffer::MemoryType(const VkPhysicalDevice& physicalDevice,
	const glm::uint32_t& typeFilter,
	const VkMemoryPropertyFlags& properties) {
	VkPhysicalDeviceMemoryProperties memProperties {};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].
		                                propertyFlags & properties) ==
		    properties) {
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
	vkGetBufferMemoryRequirements(device.GetLogical(),
		buffer,
		&memRequirements);

	const VkMemoryAllocateInfo allocInfo {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = MemoryType(device.GetPhysical(),
			memRequirements.memoryTypeBits,
			properties),
	};
	if (vkAllocateMemory(device.GetLogical(),
		    &allocInfo,
		    nullptr,
		    &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory!");
	}
	vkBindBufferMemory(device.GetLogical(), buffer, bufferMemory, 0);
}

void Buffer::CreateVertexBuffer(const Device& device,
	const Mesh& mesh,
	const Render& render) {
	const auto bufferSize = sizeof(mesh.GetVertexByIndex(0)) * mesh.
	                        GetVertices().size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory);

	void* data;
	vkMapMemory(device.GetLogical(),
		stagingBufferMemory,
		0,
		bufferSize,
		0,
		&data);
	memcpy(data, mesh.GetVertices().data(), bufferSize);
	vkUnmapMemory(device.GetLogical(), stagingBufferMemory);

	CreateBuffer(device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer,
		vertexBufferMemory);
	render.CopyCommandBuffer(device,
		mesh,
		stagingBuffer,
		vertexBuffer,
		bufferSize);

	vkDestroyBuffer(device.GetLogical(), stagingBuffer, nullptr);
	vkFreeMemory(device.GetLogical(), stagingBufferMemory, nullptr);
}

void Buffer::CreateIndexBuffer(const Device& device,
	const Mesh& mesh,
	const Render& render) {
	const auto bufferSize = sizeof(mesh.GetIndexByIndex(0)) * mesh.GetIndices().
	                        size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory);

	void* data;
	vkMapMemory(device.GetLogical(),
		stagingBufferMemory,
		0,
		bufferSize,
		0,
		&data);
	memcpy(data, mesh.GetIndices().data(), bufferSize);
	vkUnmapMemory(device.GetLogical(), stagingBufferMemory);

	CreateBuffer(device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffer,
		indexBufferMemory);

	render.CopyCommandBuffer(device,
		mesh,
		stagingBuffer,
		indexBuffer,
		bufferSize);

	vkDestroyBuffer(device.GetLogical(), stagingBuffer, nullptr);
	vkFreeMemory(device.GetLogical(), stagingBufferMemory, nullptr);
}

void Buffer::CleanupBuffers(const VkDevice& device) const {
	vkDestroyBuffer(device, indexBuffer, nullptr);
	vkFreeMemory(device, indexBufferMemory, nullptr);

	vkDestroyBuffer(device, vertexBuffer, nullptr);
	vkFreeMemory(device, vertexBufferMemory, nullptr);
}
