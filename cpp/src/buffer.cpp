#include "buffer.h"

#include <stdexcept>

#include "config.h"
#include "pipeline.h"
#include "swapchain.h"
#include "uniform.h"
#include "vertex.h"

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

void Buffer::CreateVertexBuffer(const Device& device) {
	const auto bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(
		device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	void* data;
	vkMapMemory(device.GetLogical(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), bufferSize);
	vkUnmapMemory(device.GetLogical(), stagingBufferMemory);

	CreateBuffer(
		device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer,
		vertexBufferMemory
	);

	CopyBuffer(
		device,
		stagingBuffer,
		vertexBuffer,
		bufferSize,
		commandPool
	);

	vkDestroyBuffer(device.GetLogical(), stagingBuffer, nullptr);
	vkFreeMemory(device.GetLogical(), stagingBufferMemory, nullptr);
}

void Buffer::CreateIndexBuffer(const Device& device) {
	const auto bufferSize = sizeof(indices[0]) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(
		device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	void* data;
	vkMapMemory(device.GetLogical(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), bufferSize);
	vkUnmapMemory(device.GetLogical(), stagingBufferMemory);

	CreateBuffer(
		device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffer,
		indexBufferMemory
	);

	CopyBuffer(
		device,
		stagingBuffer,
		indexBuffer,
		bufferSize,
		commandPool
	);

	vkDestroyBuffer(device.GetLogical(), stagingBuffer, nullptr);
	vkFreeMemory(device.GetLogical(), stagingBufferMemory, nullptr);
}

void Buffer::CreateCommandBuffers(const Device& device) {
	commandBuffers.resize(Config::MAX_FRAMES_IN_FLIGHT);

	const VkCommandBufferAllocateInfo allocInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = static_cast<uint32_t>(commandBuffers.size()),
	};

	if (vkAllocateCommandBuffers(
		device.GetLogical(),
		&allocInfo,
		commandBuffers.data()
	) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void Buffer::RecordCommandBuffer(
	const Pipeline& pipeline,
	const SwapChain& swapChain,
	const Descriptor& descriptor,
	const uint32_t currentFrame,
	const VkCommandBuffer commandBuffer,
	const uint32_t imageIndex
) const {
	constexpr VkCommandBufferBeginInfo beginInfo { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, };
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error(
			"failed to begin recording command buffer!"
		);
	}

	constexpr VkClearValue clearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
	VkRenderPassBeginInfo renderPassInfo {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = pipeline.GetRenderPass(),
		.framebuffer = swapChain.GetFrameBuffers()[imageIndex],
		.clearValueCount = 1,
		.pClearValues = &clearColor,
	};
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChain.GetExtent();

	vkCmdBeginRenderPass(
		commandBuffer,
		&renderPassInfo,
		VK_SUBPASS_CONTENTS_INLINE
	);

	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline.GetGraphicsPipeline()
	);

	const VkViewport viewport {
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(swapChain.GetExtent().width),
		.height = static_cast<float>(swapChain.GetExtent().height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	const VkRect2D scissor {
		.offset = { 0, 0 },
		.extent = swapChain.GetExtent(),
	};
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	const VkBuffer vertexBuffers[] = { vertexBuffer };
	constexpr VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(
		commandBuffer,
		indexBuffer,
		0,
		VK_INDEX_TYPE_UINT16
	);

	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline.GetPipelineLayout(),
		0,
		1,
		&descriptor.GetDescriptorSets()[currentFrame],
		0,
		nullptr
	);

	vkCmdDrawIndexed(
		commandBuffer,
		static_cast<uint32_t>(indices.size()),
		1,
		0,
		0,
		0
	);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}
