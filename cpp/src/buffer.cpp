#include "buffer.h"

#include <stdexcept>

uint32_t Buffer::MemoryType(
	const VkPhysicalDevice& physicalDevice,
	const glm::uint32_t& typeFilter,
	const VkMemoryPropertyFlags& properties
) {
	VkPhysicalDeviceMemoryProperties memProperties {};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].
			propertyFlags & properties) == properties) { return i; }
	}
	throw std::runtime_error("Failed to find suitable memory type!");
}

void Buffer::CreateBuffer(
	const Device& device,
	const VkDeviceSize& size,
	const VkBufferUsageFlags& usage,
	const VkMemoryPropertyFlags& properties,
	VkBuffer& buffer,
	VkDeviceMemory& bufferMemory
) {
	const VkBufferCreateInfo bufferInfo {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	if (vkCreateBuffer(device.GetLogical(), &bufferInfo, nullptr, &buffer) !=
		VK_SUCCESS) { throw std::runtime_error("Failed to create buffer!"); }

	VkMemoryRequirements memRequirements {};
	vkGetBufferMemoryRequirements(device.GetLogical(), buffer, &memRequirements);

	const VkMemoryAllocateInfo allocInfo {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = MemoryType(
			device.GetPhysical(),
			memRequirements.memoryTypeBits,
			properties
		),
	};
	if (vkAllocateMemory(device.GetLogical(), &allocInfo, nullptr, &bufferMemory) !=
		VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory!");
	}
	vkBindBufferMemory(device.GetLogical(), buffer, bufferMemory, 0);
}

void Buffer::CopyBuffer(
	const Device& device,
	const VkBuffer& srcBuffer,
	const VkBuffer& dstBuffer,
	const VkDeviceSize& size,
	const VkCommandPool& commandPool
) {
	const VkCommandBufferAllocateInfo allocInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
	VkCommandBuffer commandBuffer {};
	vkAllocateCommandBuffers(device.GetLogical(), &allocInfo, &commandBuffer);
	constexpr VkCommandBufferBeginInfo beginInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	const VkBufferCopy copyRegion { .size = size, };
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	vkEndCommandBuffer(commandBuffer);
	const VkSubmitInfo submitInfo {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
	};
	const auto& graphicsQueue = device.GetGraphicsQueue();
	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);
	vkFreeCommandBuffers(device.GetLogical(), commandPool, 1, &commandBuffer);
}
