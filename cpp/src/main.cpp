#define GLM_FORCE_RADIANS // NOLINT(clang-diagnostic-unused-macros)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>
#include <GLFW/glfw3.h>

#include "buffer.h"
#include "uniform.h"
#include "device.h"
#include "pipeline.h"
#include "shader.h"
#include "swapchain.h"
#include "validation.h"
#include "vertex.h"
#include "window.h"

const std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0 };

class HelloTriangleApplication {
	Shader     shader;
	Device     device;
	Window     window;
	Pipeline   pipeline;
	SwapChain  swapChain;
	Descriptor descriptor;
	Validation validation;

public:
	auto Run() -> void {
		window.CreateWindow();
		InitVulkan();
		MainLoop();
		Cleanup();
	}

	auto MainLoop() -> void {
		while (!glfwWindowShouldClose(window.window)) {
			glfwPollEvents();
			drawFrame();
		}
		device.WaitIdle();
	}

private:
	VkInstance instance;

	VkCommandPool commandPool;

	VkBuffer       vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer       indexBuffer;
	VkDeviceMemory indexBufferMemory;

	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence>     inFlightFences;
	uint32_t                 currentFrame = 0;

	auto InitVulkan() -> void {
		CreateInstance();

		validation.SetupMessenger(instance);

		window.CreateSurface(instance);

		device.PickPhysicalDevice(instance, window.GetSurface());
		device.CreateLogicalDevice(window.GetSurface(), validation);

		swapChain.Create(device, window);
		swapChain.CreateImageViews(device.Get());

		pipeline.CreateRenderPass(swapChain.GetImageFormat(), device.Get());

		descriptor.CreateDescriptorSetLayout(device.Get());

		pipeline.CreateGraphicsPipeline(
			shader,
			device.Get(),
			descriptor.GetSetLayout()
		);

		swapChain.CreateFrameBuffers(device.Get(), pipeline.GetRenderPass());

		createCommandPool();
		createVertexBuffer();
		createIndexBuffer();
		descriptor.CreateUniformBuffers(device);
		descriptor.CreateDescriptorPool(device.Get());
		descriptor.CreateDescriptorSets(device.Get());

		createCommandBuffers();
		createSyncObjects();
	}

	auto Cleanup() const -> void {
		swapChain.CleanupSwapChain(device.Get());
		pipeline.DestroyGraphicsPipeline(device.Get());

		descriptor.Destroy(device.Get());

		vkDestroyBuffer(device.Get(), indexBuffer, nullptr);
		vkFreeMemory(device.Get(), indexBufferMemory, nullptr);

		vkDestroyBuffer(device.Get(), vertexBuffer, nullptr);
		vkFreeMemory(device.Get(), vertexBufferMemory, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(
				device.Get(),
				renderFinishedSemaphores[i],
				nullptr
			);
			vkDestroySemaphore(
				device.Get(),
				imageAvailableSemaphores[i],
				nullptr
			);
			vkDestroyFence(device.Get(), inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(device.Get(), commandPool, nullptr);

		vkDestroyDevice(device.Get(), nullptr);

		if (validation.GetEnabled()) {
			Validation::DestroyMessengerEXT(
				instance,
				validation.GetDebugMessenger(),
				nullptr
			);
		}

		vkDestroySurfaceKHR(instance, window.GetSurface(), nullptr);
		vkDestroyInstance(instance, nullptr);

		window.DestroyWindow();
	}

	auto RecreateSwapChain() -> void {
		window.OnRecreateSwapChain();
		device.WaitIdle();

		swapChain.CleanupSwapChain(device.Get());

		swapChain.Create(device, window);
		swapChain.CreateImageViews(device.Get());
		swapChain.CreateFrameBuffers(device.Get(), pipeline.GetRenderPass());
	}

	auto CreateInstance() -> void {
		if (validation.GetEnabled() && !validation.CheckLayerSupport()) {
			throw std::runtime_error(
				"validation layers requested, but not available!"
			);
		}

		VkApplicationInfo appInfo {};
		appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName   = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName        = "No Engine";
		appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion         = VK_API_VERSION_1_3;

		VkInstanceCreateInfo createInfo {};
		createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		const auto extensions            = GetRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.
			size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (validation.GetEnabled()) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validation.
				GetLayers().size());
			createInfo.ppEnabledLayerNames = validation.GetLayers().data();

			debugCreateInfo  = validation.GetMessengerCreateInfo();
			createInfo.pNext = &debugCreateInfo;
		} else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext             = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}

	auto createCommandPool() -> void {
		auto queueFamilyIndices = device.FindQueueFamilies(window.GetSurface());

		VkCommandPoolCreateInfo poolInfo {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(device.Get(), &poolInfo, nullptr, &commandPool)
			!= VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics command pool!");
		}
	}

	auto createVertexBuffer() -> void {
		auto bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer       stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Buffer::CreateBuffer(
			device,
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		void* data;
		vkMapMemory(device.Get(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), bufferSize);
		vkUnmapMemory(device.Get(), stagingBufferMemory);

		Buffer::CreateBuffer(
			device,
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBuffer,
			vertexBufferMemory
		);

		Buffer::CopyBuffer(
			device,
			stagingBuffer,
			vertexBuffer,
			bufferSize,
			commandPool
		);

		vkDestroyBuffer(device.Get(), stagingBuffer, nullptr);
		vkFreeMemory(device.Get(), stagingBufferMemory, nullptr);
	}

	auto createIndexBuffer() -> void {
		auto bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer       stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Buffer::CreateBuffer(
			device,
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		void* data;
		vkMapMemory(device.Get(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), bufferSize);
		vkUnmapMemory(device.Get(), stagingBufferMemory);

		Buffer::CreateBuffer(
			device,
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBuffer,
			indexBufferMemory
		);

		Buffer::CopyBuffer(
			device,
			stagingBuffer,
			indexBuffer,
			bufferSize,
			commandPool
		);

		vkDestroyBuffer(device.Get(), stagingBuffer, nullptr);
		vkFreeMemory(device.Get(), stagingBufferMemory, nullptr);
	}

	auto createCommandBuffers() -> void {
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.
			size());

		if (vkAllocateCommandBuffers(
			device.Get(),
			&allocInfo,
			commandBuffers.data()
		) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	auto recordCommandBuffer(
		VkCommandBuffer commandBuffer,
		uint32_t        imageIndex
	) -> void {
		VkCommandBufferBeginInfo beginInfo {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error(
				"failed to begin recording command buffer!"
			);
		}

		VkRenderPassBeginInfo renderPassInfo {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = pipeline.GetRenderPass();
		renderPassInfo.framebuffer = swapChain.GetFrameBuffers()[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChain.GetExtent();

		VkClearValue clearColor        = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues    = &clearColor;

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

		VkViewport viewport {};
		viewport.x        = 0.0f;
		viewport.y        = 0.0f;
		viewport.width    = static_cast<float>(swapChain.GetExtent().width);
		viewport.height   = static_cast<float>(swapChain.GetExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor {};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChain.GetExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		VkBuffer     vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[]       = { 0 };
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

	auto createSyncObjects() -> void {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(
				device.Get(),
				&semaphoreInfo,
				nullptr,
				&imageAvailableSemaphores[i]
			) != VK_SUCCESS || vkCreateSemaphore(
				device.Get(),
				&semaphoreInfo,
				nullptr,
				&renderFinishedSemaphores[i]
			) != VK_SUCCESS || vkCreateFence(
				device.Get(),
				&fenceInfo,
				nullptr,
				&inFlightFences[i]
			) != VK_SUCCESS) {
				throw std::runtime_error(
					"failed to create synchronization objects for a frame!"
				);
			}
		}
	}

	auto drawFrame() -> void {
		vkWaitForFences(
			device.Get(),
			1,
			&inFlightFences[currentFrame],
			VK_TRUE,
			UINT64_MAX
		);

		uint32_t imageIndex;
		auto     result = vkAcquireNextImageKHR(
			device.Get(),
			swapChain.Get(),
			UINT64_MAX,
			imageAvailableSemaphores[currentFrame],
			VK_NULL_HANDLE,
			&imageIndex
		);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapChain();
			return;
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		descriptor.UpdateUniformBuffers(swapChain.GetExtent(), currentFrame);

		vkResetFences(device.Get(), 1, &inFlightFences[currentFrame]);

		vkResetCommandBuffer(
			commandBuffers[currentFrame],
			/*VkCommandBufferResetFlagBits*/
			0
		);
		recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

		VkSubmitInfo submitInfo {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = {
			imageAvailableSemaphores[currentFrame]
		};
		VkPipelineStageFlags waitStages[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores    = waitSemaphores;
		submitInfo.pWaitDstStageMask  = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers    = &commandBuffers[currentFrame];

		VkSemaphore signalSemaphores[] = {
			renderFinishedSemaphores[currentFrame]
		};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores    = signalSemaphores;

		if (vkQueueSubmit(
			device.GetGraphicsQueue(),
			1,
			&submitInfo,
			inFlightFences[currentFrame]
		) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores    = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain.Get() };
		presentInfo.swapchainCount  = 1;
		presentInfo.pSwapchains     = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(device.GetPresentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
			window.GetFrameBufferResized()) {
			window.SetFrameBufferResized(false);
			RecreateSwapChain();
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	auto GetRequiredExtensions() const -> std::vector<const char*> {
		const auto& [glfwExtensions, glfwExtensionCount] =
			Window::GetRequiredExtensions();
		std::vector extensions(
			glfwExtensions,
			glfwExtensions + glfwExtensionCount
		);
		if (validation.GetEnabled()) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}
};

auto main() -> int {
	HelloTriangleApplication app;

	try { app.Run(); } catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
