#include <stdexcept>

#include "buffer.h"
#include "device.h"

auto Buffer::FindMemoryType(
	const VkPhysicalDevice&      physicalDevice,
	const glm::uint32_t&         typeFilter,
	const VkMemoryPropertyFlags& properties
) -> uint32_t {

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].
			propertyFlags & properties) == properties) { return i; }
	}
	throw std::runtime_error("failed to find suitable memory type!");
}

auto Buffer::CreateBuffer(
	const Device&         device,
	VkDeviceSize          size,
	VkBufferUsageFlags    usage,
	VkMemoryPropertyFlags properties,
	VkBuffer&             buffer,
	VkDeviceMemory&       bufferMemory
) -> void {
	VkBufferCreateInfo bufferInfo {};
	bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size        = size;
	bufferInfo.usage       = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device.Get(), &bufferInfo, nullptr, &buffer) !=
		VK_SUCCESS) { throw std::runtime_error("failed to create buffer!"); }

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device.Get(), buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo {};
	allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize  = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(
		device.GetPhysical(),
		memRequirements.memoryTypeBits,
		properties
	);

	if (vkAllocateMemory(device.Get(), &allocInfo, nullptr, &bufferMemory) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(device.Get(), buffer, bufferMemory, 0);
}

auto Buffer::CopyBuffer(
	const Device&        device,
	VkBuffer             srcBuffer,
	VkBuffer             dstBuffer,
	VkDeviceSize         size,
	const VkCommandPool& commandPool
) -> void {
	VkCommandBufferAllocateInfo allocInfo {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device.Get(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo {};
	submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers    = &commandBuffer;

	vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(device.GetGraphicsQueue());

	vkFreeCommandBuffers(device.Get(), commandPool, 1, &commandBuffer);
}
