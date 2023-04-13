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

#include "device.h"
#include "shader.h"
#include "validation.h"
#include "window.h"

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;

	static auto GetBindingDescription() -> VkVertexInputBindingDescription {
		VkVertexInputBindingDescription bindingDescription {};
		bindingDescription.binding   = 0;
		bindingDescription.stride    = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static auto
	GetAttributeDescriptions() -> std::array<VkVertexInputAttributeDescription,
		2> {
		const std::array attributeDescriptions {
			VkVertexInputAttributeDescription {
				.location = 0,
				.binding = 0,
				.format = VK_FORMAT_R32G32_SFLOAT,
				.offset = offsetof(Vertex, pos),
			},
			VkVertexInputAttributeDescription {
				.location = 1,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex, color),
			}
		};
		return attributeDescriptions;
	}
};

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

const Vertexes vertices = {
	{ { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
	{ { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
	{ { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
	{ { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } }
};

const std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0 };

class HelloTriangleApplication {
	Shader     shader;
	Device     device;
	Window     window;
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
	VkInstance               instance;
	

	VkSwapchainKHR             swapChain;
	std::vector<VkImage>       swapChainImages;
	VkFormat                   swapChainImageFormat;
	VkExtent2D                 swapChainExtent;
	std::vector<VkImageView>   swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkRenderPass          renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout      pipelineLayout;
	VkPipeline            graphicsPipeline;

	VkCommandPool commandPool;

	VkBuffer       vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer       indexBuffer;
	VkDeviceMemory indexBufferMemory;

	std::vector<VkBuffer>       uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*>          uniformBuffersMapped;

	VkDescriptorPool             descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

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

		createSwapChain();
		createImageViews();
		createRenderPass();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		createVertexBuffer();
		createIndexBuffer();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
		createSyncObjects();
	}

	

	auto CleanupSwapChain() const -> void {
		for (auto framebuffer: swapChainFramebuffers) {
			vkDestroyFramebuffer(device.Get(), framebuffer, nullptr);
		}

		for (auto imageView: swapChainImageViews) {
			vkDestroyImageView(device.Get(), imageView, nullptr);
		}

		vkDestroySwapchainKHR(device.Get(), swapChain, nullptr);
	}

	auto Cleanup() const -> void {
		CleanupSwapChain();

		vkDestroyPipeline(device.Get(), graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device.Get(), pipelineLayout, nullptr);
		vkDestroyRenderPass(device.Get(), renderPass, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(device.Get(), uniformBuffers[i], nullptr);
			vkFreeMemory(device.Get(), uniformBuffersMemory[i], nullptr);
		}

		vkDestroyDescriptorPool(device.Get(), descriptorPool, nullptr);

		vkDestroyDescriptorSetLayout(
			device.Get(),
			descriptorSetLayout,
			nullptr
		);

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
			Validation::DestroyMessengerEXT(instance, validation.GetDebugMessenger(), nullptr);
		}

		vkDestroySurfaceKHR(instance, window.GetSurface(), nullptr);
		vkDestroyInstance(instance, nullptr);

		window.DestroyWindow();
	}

	auto RecreateSwapChain() -> void {
		window.OnRecreateSwapChain();
		device.WaitIdle();

		CleanupSwapChain();

		createSwapChain();
		createImageViews();
		createFramebuffers();
	}

	auto CreateInstance() -> void {
		if (validation.GetEnabled() && !validation.
			CheckLayerSupport()) {
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

	

	auto createSwapChain() -> void {
		auto swapChainSupport = device.QuerySwapChainSupport(window.GetSurface());

		auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		auto extent = chooseSwapExtent(swapChainSupport.capabilities);

		auto imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount >
			swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo {};
		createInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = window.GetSurface();

		createInfo.minImageCount    = imageCount;
		createInfo.imageFormat      = surfaceFormat.format;
		createInfo.imageColorSpace  = surfaceFormat.colorSpace;
		createInfo.imageExtent      = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		auto     indices              = device.FindQueueFamilies(window.GetSurface());
		uint32_t queueFamilyIndices[] = {
			indices.graphicsFamily.value(),
			indices.presentFamily.value()
		};

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices   = queueFamilyIndices;
		} else { createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; }

		createInfo.preTransform = swapChainSupport.capabilities.
		                                           currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode    = presentMode;
		createInfo.clipped        = VK_TRUE;

		if (vkCreateSwapchainKHR(device.Get(), &createInfo, nullptr, &swapChain)
			!= VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(device.Get(), swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(
			device.Get(),
			swapChain,
			&imageCount,
			swapChainImages.data()
		);

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent      = extent;
	}

	auto createImageViews() -> void {
		swapChainImageViews.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			VkImageViewCreateInfo createInfo {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(
				device.Get(),
				&createInfo,
				nullptr,
				&swapChainImageViews[i]
			) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}
		}
	}

	auto createRenderPass() -> void {
		VkAttachmentDescription colorAttachment {};
		colorAttachment.format         = swapChainImageFormat;
		colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass {};
		subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments    = &colorAttachmentRef;

		VkSubpassDependency dependency {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(
			device.Get(),
			&renderPassInfo,
			nullptr,
			&renderPass
		) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	auto createDescriptorSetLayout() -> void {
		VkDescriptorSetLayoutBinding uboLayoutBinding {};
		uboLayoutBinding.binding            = 0;
		uboLayoutBinding.descriptorCount    = 1;
		uboLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(
			device.Get(),
			&layoutInfo,
			nullptr,
			&descriptorSetLayout
		) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	auto createGraphicsPipeline() -> void {
		auto shaderStages(shader.AutoCreateStages(device.Get()));

		VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
		vertexInputInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		auto bindingDescription    = Vertex::GetBindingDescription();
		auto attributeDescriptions = Vertex::GetAttributeDescriptions();

		vertexInputInfo.vertexBindingDescriptionCount   = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(
			attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions   = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.
			data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
		inputAssembly.sType =
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState {};
		viewportState.sType =
			VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount  = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer {};
		rasterizer.sType =
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable        = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth               = 1.0f;
		rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable         = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling {};
		multisampling.sType =
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable  = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending {};
		colorBlending.sType =
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable     = VK_FALSE;
		colorBlending.logicOp           = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount   = 1;
		colorBlending.pAttachments      = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState {};
		dynamicState.sType =
			VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.
			size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
		pipelineLayoutInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts    = &descriptorSetLayout;

		if (vkCreatePipelineLayout(
			device.Get(),
			&pipelineLayoutInfo,
			nullptr,
			&pipelineLayout
		) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(
			device.Get(),
			VK_NULL_HANDLE,
			1,
			&pipelineInfo,
			nullptr,
			&graphicsPipeline
		) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}
		shader.DestroyModules(device.Get());
	}

	auto createFramebuffers() -> void {
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			VkImageView attachments[] = { swapChainImageViews[i] };

			VkFramebufferCreateInfo framebufferInfo {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(
				device.Get(),
				&framebufferInfo,
				nullptr,
				&swapChainFramebuffers[i]
			) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
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
		createBuffer(
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

		createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBuffer,
			vertexBufferMemory
		);

		copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(device.Get(), stagingBuffer, nullptr);
		vkFreeMemory(device.Get(), stagingBufferMemory, nullptr);
	}

	auto createIndexBuffer() -> void {
		auto bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer       stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(
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

		createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBuffer,
			indexBufferMemory
		);

		copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(device.Get(), stagingBuffer, nullptr);
		vkFreeMemory(device.Get(), stagingBufferMemory, nullptr);
	}

	auto createUniformBuffers() -> void {
		auto bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			createBuffer(
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				uniformBuffers[i],
				uniformBuffersMemory[i]
			);

			vkMapMemory(
				device.Get(),
				uniformBuffersMemory[i],
				0,
				bufferSize,
				0,
				&uniformBuffersMapped[i]
			);
		}
	}

	auto createDescriptorPool() -> void {
		VkDescriptorPoolSize poolSize {};
		poolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolCreateInfo poolInfo {};
		poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes    = &poolSize;
		poolInfo.maxSets       = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(
			device.Get(),
			&poolInfo,
			nullptr,
			&descriptorPool
		) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	auto createDescriptorSets() -> void {
		std::vector<VkDescriptorSetLayout> layouts(
			MAX_FRAMES_IN_FLIGHT,
			descriptorSetLayout
		);
		VkDescriptorSetAllocateInfo allocInfo {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(
			MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(
			device.Get(),
			&allocInfo,
			descriptorSets.data()
		) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkDescriptorBufferInfo bufferInfo {};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range  = sizeof(UniformBufferObject);

			VkWriteDescriptorSet descriptorWrite {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(
				device.Get(),
				1,
				&descriptorWrite,
				0,
				nullptr
			);
		}
	}

	auto createBuffer(
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
			VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device.Get(), buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo {};
		allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize  = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(
			memRequirements.memoryTypeBits,
			properties
		);

		if (vkAllocateMemory(device.Get(), &allocInfo, nullptr, &bufferMemory)
			!= VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(device.Get(), buffer, bufferMemory, 0);
	}

	auto copyBuffer(
		VkBuffer     srcBuffer,
		VkBuffer     dstBuffer,
		VkDeviceSize size
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

		vkQueueSubmit(
			device.GetGraphicsQueue(),
			1,
			&submitInfo,
			VK_NULL_HANDLE
		);
		vkQueueWaitIdle(device.GetGraphicsQueue());

		vkFreeCommandBuffers(device.Get(), commandPool, 1, &commandBuffer);
	}

	auto findMemoryType(
		uint32_t              typeFilter,
		VkMemoryPropertyFlags properties
	) -> uint32_t {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(
			device.GetPhysical(),
			&memProperties
		);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].
				propertyFlags & properties) == properties) { return i; }
		}

		throw std::runtime_error("failed to find suitable memory type!");
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
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

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
			graphicsPipeline
		);

		VkViewport viewport {};
		viewport.x        = 0.0f;
		viewport.y        = 0.0f;
		viewport.width    = static_cast<float>(swapChainExtent.width);
		viewport.height   = static_cast<float>(swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor {};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;
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
			pipelineLayout,
			0,
			1,
			&descriptorSets[currentFrame],
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

	auto updateUniformBuffer(uint32_t currentImage) -> void {
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		auto time = std::chrono::duration<float, std::chrono::seconds::period>(
			currentTime - startTime
		).count();

		UniformBufferObject ubo {};
		ubo.model = rotate(
			glm::mat4(1.0f),
			time * glm::radians(90.0f),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);
		ubo.view = lookAt(
			glm::vec3(2.0f, 2.0f, 2.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);
		ubo.proj = glm::perspective(
			glm::radians(45.0f),
			swapChainExtent.width / static_cast<float>(swapChainExtent.height),
			0.1f,
			10.0f
		);
		ubo.proj[1][1] *= -1;

		memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
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
			swapChain,
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

		updateUniformBuffer(currentFrame);

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

		VkSwapchainKHR swapChains[] = { swapChain };
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

	auto chooseSwapSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR>& availableFormats
	) -> VkSurfaceFormatKHR {
		for (const auto& availableFormat: availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace ==
				VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) { return availableFormat; }
		}

		return availableFormats[0];
	}

	auto chooseSwapPresentMode(
		const std::vector<VkPresentModeKHR>& availablePresentModes
	) -> VkPresentModeKHR {
		for (const auto& availablePresentMode: availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	auto chooseSwapExtent(
		const VkSurfaceCapabilitiesKHR& capabilities
	) -> VkExtent2D {
		if (capabilities.currentExtent.width != std::numeric_limits<
			uint32_t>::max()) { return capabilities.currentExtent; }

		auto [width, height] = window.GetFrameBufferSize();

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(
			actualExtent.width,
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width
		);
		actualExtent.height = std::clamp(
			actualExtent.height,
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height
		);

		return actualExtent;
	}

	auto GetRequiredExtensions() const -> std::vector<const char*> {
		const auto& [glfwExtensions, glfwExtensionCount] = Window::GetRequiredExtensions();
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
