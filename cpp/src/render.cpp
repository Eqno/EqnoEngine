#include "render.h"

#include <stdexcept>

#include "buffer.h"
#include "device.h"
#include "swapchain.h"
#include "uniform.h"
#include "vertex.h"

void Render::CreateCommandPool(const Device& device,
	const VkSurfaceKHR& surface) {
	const auto [graphicsFamily, presentFamily] = device.
		FindQueueFamilies(surface);

	const VkCommandPoolCreateInfo poolInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = graphicsFamily.has_value() ? graphicsFamily.value()
		: 0,
	};

	if (vkCreateCommandPool(device.GetLogical(),
		    &poolInfo,
		    nullptr,
		    &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics command pool!");
	}
}

void Render::DestroyCommandPool(const VkDevice& device) const {
	vkDestroyCommandPool(device, commandPool, nullptr);
}

void Render::CreateCommandBuffers(const VkDevice& device,
	int maxFramesInFlight) {
	commandBuffers.resize(maxFramesInFlight);

	const VkCommandBufferAllocateInfo allocInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = static_cast<uint32_t>(commandBuffers.size()),
	};

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void Render::RecordCommandBuffer(const VkBuffer& indexBuffer,
	const VkBuffer& vertexBuffer,
	const Mesh& mesh,
	const Pipeline& pipeline,
	const SwapChain& swapChain,
	const Descriptor& descriptor,
	const uint32_t imageIndex) const {
	const VkCommandBuffer commandBuffer = commandBuffers[currentFrame];

	constexpr VkCommandBufferBeginInfo beginInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}
	constexpr std::array clearValues {
		VkClearValue {.color = {{0.0f, 0.0f, 0.0f, 1.0f}}},
		VkClearValue {.depthStencil = {1.0f, 0}},
	};
	VkRenderPassBeginInfo renderPassInfo {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = pipeline.GetRenderPass(),
		.framebuffer = swapChain.GetFrameBuffers()[imageIndex],
		.clearValueCount = static_cast<uint32_t>(clearValues.size()),
		.pClearValues = clearValues.data(),
	};
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = swapChain.GetExtent();

	vkCmdBeginRenderPass(commandBuffer,
		&renderPassInfo,
		VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline.GetGraphicsPipeline());

	const VkViewport viewport {
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(swapChain.GetExtent().width),
		.height = static_cast<float>(swapChain.GetExtent().height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	const VkRect2D scissor {.offset = {0, 0}, .extent = swapChain.GetExtent(),};
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	const VkBuffer vertexBuffers[] = {vertexBuffer};
	constexpr VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline.GetPipelineLayout(),
		0,
		1,
		&descriptor.GetDescriptorSets()[currentFrame],
		0,
		nullptr);

	vkCmdDrawIndexed(commandBuffer,
		static_cast<uint32_t>(mesh.GetIndices().size()),
		1,
		0,
		0,
		0);
	vkCmdEndRenderPass(commandBuffer);
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void Render::CopyCommandBuffer(const Device& device,
	const Mesh& mesh,
	const VkBuffer& srcBuffer,
	const VkBuffer& dstBuffer,
	const VkDeviceSize& size) const {
	const VkCommandBuffer commandBuffer = BeginSingleTimeCommands(
		device.GetLogical());
	const VkBufferCopy copyRegion {.size = size,};
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	EndSingleTimeCommands(device, mesh, commandBuffer);
}

VkCommandBuffer Render::BeginSingleTimeCommands(const VkDevice& device) const {
	const VkCommandBufferAllocateInfo allocInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
	VkCommandBuffer commandBuffer {};
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	constexpr VkCommandBufferBeginInfo beginInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}

void Render::EndSingleTimeCommands(const Device& device, const Mesh& mesh,
	VkCommandBuffer commandBuffer) const {
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

void Render::DrawFrame(const VkBuffer& indexBuffer,
	const VkBuffer& vertexBuffer,
	const Device& device,
	const Mesh& mesh,
	Depth& depth,
	Window& window,
	const Pipeline& pipeline,
	SwapChain& swapChain,
	const Descriptor& descriptor) {
	vkWaitForFences(device.GetLogical(),
		1,
		&inFlightFences[currentFrame],
		VK_TRUE,
		UINT64_MAX);

	uint32_t imageIndex;
	auto result = vkAcquireNextImageKHR(device.GetLogical(),
		swapChain.Get(),
		UINT64_MAX,
		imageAvailableSemaphores[currentFrame],
		VK_NULL_HANDLE,
		&imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		swapChain.RecreateSwapChain(device, depth, window, pipeline);
		return;
	}
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	descriptor.UpdateUniformBuffers(swapChain.GetExtent(), currentFrame);

	vkResetFences(device.GetLogical(), 1, &inFlightFences[currentFrame]);

	vkResetCommandBuffer(commandBuffers[currentFrame],
		/*VkCommandBufferResetFlagBits*/
		0);

	RecordCommandBuffer(indexBuffer,
		vertexBuffer,
		mesh,
		pipeline,
		swapChain,
		descriptor,
		imageIndex);

	const VkSemaphore waitSemaphores[] = {
		imageAvailableSemaphores[currentFrame]
	};
	constexpr VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};
	const VkSemaphore signalSemaphores[] = {
		renderFinishedSemaphores[currentFrame]
	};

	const VkSubmitInfo submitInfo {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = waitSemaphores,
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffers[currentFrame],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signalSemaphores,
	};

	if (vkQueueSubmit(device.GetGraphicsQueue(),
		    1,
		    &submitInfo,
		    inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	const VkSwapchainKHR swapChains[] = {swapChain.Get()};
	const VkPresentInfoKHR presentInfo {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signalSemaphores,
		.swapchainCount = 1,
		.pSwapchains = swapChains,
		.pImageIndices = &imageIndex,
	};
	result = vkQueuePresentKHR(device.GetPresentQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
	    window.GetFrameBufferResized()) {
		window.SetFrameBufferResized(false);
		swapChain.RecreateSwapChain(device, depth, window, pipeline);
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}
	currentFrame = (currentFrame + 1) % descriptor.GetUniformBuffer().
	               GetMaxFramesInFlight();
}

void Render::CreateSyncObjects(const VkDevice& device,
	const int maxFramesInFlight) {
	imageAvailableSemaphores.resize(maxFramesInFlight);
	renderFinishedSemaphores.resize(maxFramesInFlight);
	inFlightFences.resize(maxFramesInFlight);

	constexpr VkSemaphoreCreateInfo semaphoreInfo {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	constexpr VkFenceCreateInfo fenceInfo {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	for (size_t i = 0; i < maxFramesInFlight; i++) {
		if (vkCreateSemaphore(device,
			    &semaphoreInfo,
			    nullptr,
			    &imageAvailableSemaphores[i]) != VK_SUCCESS ||
		    vkCreateSemaphore(device,
			    &semaphoreInfo,
			    nullptr,
			    &renderFinishedSemaphores[i]) != VK_SUCCESS || vkCreateFence(
			    device,
			    &fenceInfo,
			    nullptr,
			    &inFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error(
				"failed to create synchronization objects for a frame!");
		}
	}
}

void Render::DestroySyncObjects(const VkDevice& device,
	const int maxFramesInFlight) const {
	for (size_t i = 0; i < maxFramesInFlight; i++) {
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}
}
