#include <iostream>
#include <stdexcept>

#include "buffer.h"
#include "device.h"
#include "pipeline.h"
#include "shader.h"
#include "swapchain.h"
#include "uniform.h"
#include "validation.h"
#include "vertex.h"
#include "window.h"


class HelloTriangleApplication {
	Shader shader;
	Device device;
	Window window;
	Buffer buffer;
	Pipeline pipeline;
	SwapChain swapChain;
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
			DrawFrame();
		}
		device.WaitIdle();
	}

private:
	VkInstance instance;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	uint32_t currentFrame = 0;

	auto InitVulkan() -> void {
		CreateInstance();

		validation.SetupMessenger(instance);

		window.CreateSurface(instance);

		device.PickPhysicalDevice(instance, window.GetSurface());
		device.CreateLogicalDevice(window.GetSurface(), validation);

		swapChain.Create(device, window);
		swapChain.CreateImageViews(device.GetLogical());

		pipeline.CreateRenderPass(swapChain.GetImageFormat(), device.GetLogical());

		descriptor.CreateDescriptorSetLayout(device.GetLogical());

		pipeline.CreateGraphicsPipeline(
			shader,
			device.GetLogical(),
			descriptor.GetSetLayout()
		);

		swapChain.CreateFrameBuffers(device.GetLogical(), pipeline.GetRenderPass());

		CreateCommandPool();
		buffer.CreateVertexBuffer(device);
		buffer.CreateIndexBuffer(device);
		descriptor.CreateUniformBuffers(device);
		descriptor.CreateDescriptorPool(device.GetLogical());
		descriptor.CreateDescriptorSets(device.GetLogical());

		buffer.CreateCommandBuffers(device);
		CreateSyncObjects();
	}

	void Cleanup() {
		swapChain.CleanupSwapChain(device.GetLogical());
		pipeline.DestroyGraphicsPipeline(device.GetLogical());

		descriptor.Destroy(device.GetLogical());

		vkDestroyBuffer(device.GetLogical(), buffer.GetIndexBuffer(), nullptr);
		vkFreeMemory(device.GetLogical(), buffer.GetIndexBufferMemory(), nullptr);

		vkDestroyBuffer(device.GetLogical(), buffer.GetVertexBuffer(), nullptr);
		vkFreeMemory(device.GetLogical(), buffer.GetVertexBufferMemory(), nullptr);

		for (size_t i = 0; i < Config::MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(
				device.GetLogical(),
				renderFinishedSemaphores[i],
				nullptr
			);
			vkDestroySemaphore(
				device.GetLogical(),
				imageAvailableSemaphores[i],
				nullptr
			);
			vkDestroyFence(device.GetLogical(), inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(device.GetLogical(), buffer.GetCommandPool(), nullptr);

		vkDestroyDevice(device.GetLogical(), nullptr);

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

	void RecreateSwapChain() {
		window.OnRecreateSwapChain();
		device.WaitIdle();

		swapChain.CleanupSwapChain(device.GetLogical());

		swapChain.Create(device, window);
		swapChain.CreateImageViews(device.GetLogical());
		swapChain.CreateFrameBuffers(device.GetLogical(), pipeline.GetRenderPass());
	}

	void CreateInstance() {
		if (validation.GetEnabled() && !validation.CheckLayerSupport()) {
			throw std::runtime_error(
				"validation layers requested, but not available!"
			);
		}

		VkApplicationInfo appInfo {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo createInfo {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		const auto extensions = GetRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.
			size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (validation.GetEnabled()) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validation.
			                                                     GetLayers().size());
			createInfo.ppEnabledLayerNames = validation.GetLayers().data();

			debugCreateInfo = validation.GetMessengerCreateInfo();
			createInfo.pNext = &debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}

	void CreateCommandPool() {
		auto queueFamilyIndices = device.FindQueueFamilies(window.GetSurface());

		VkCommandPoolCreateInfo poolInfo {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(device.GetLogical(), &poolInfo, nullptr, &buffer.GetCommandPool())
			!= VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics command pool!");
		}
	}


	void CreateSyncObjects() {
		imageAvailableSemaphores.resize(Config::MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(Config::MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(Config::MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < Config::MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(
				device.GetLogical(),
				&semaphoreInfo,
				nullptr,
				&imageAvailableSemaphores[i]
			) != VK_SUCCESS || vkCreateSemaphore(
				device.GetLogical(),
				&semaphoreInfo,
				nullptr,
				&renderFinishedSemaphores[i]
			) != VK_SUCCESS || vkCreateFence(
				device.GetLogical(),
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

	void DrawFrame() {
		vkWaitForFences(
			device.GetLogical(),
			1,
			&inFlightFences[currentFrame],
			VK_TRUE,
			UINT64_MAX
		);

		uint32_t imageIndex;
		auto result = vkAcquireNextImageKHR(
			device.GetLogical(),
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

		vkResetFences(device.GetLogical(), 1, &inFlightFences[currentFrame]);

		vkResetCommandBuffer(
			buffer.GetCommandBufferByFrame(currentFrame),
			/*VkCommandBufferResetFlagBits*/
			0
		);
		buffer.RecordCommandBuffer(pipeline, swapChain, descriptor, currentFrame,
			buffer.GetCommandBufferByFrame(currentFrame), imageIndex);

		VkSubmitInfo submitInfo {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		const VkSemaphore waitSemaphores[] = {
			imageAvailableSemaphores[currentFrame]
		};
		constexpr VkPipelineStageFlags waitStages[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &buffer.GetCommandBufferByFrame(currentFrame);

		VkSemaphore signalSemaphores[] = {
			renderFinishedSemaphores[currentFrame]
		};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

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
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain.Get() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(device.GetPresentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
			window.GetFrameBufferResized()) {
			window.SetFrameBufferResized(false);
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		currentFrame = (currentFrame + 1) % Config::MAX_FRAMES_IN_FLIGHT;
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

int main() {
	try {
		HelloTriangleApplication app;
		app.Run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
