#include "../include/vulkan.h"

void Vulkan::Run() {
	window.CreateWindow();
	InitVulkan();
	MainLoop();
	Cleanup();
}

void Vulkan::MainLoop() {
	while (!glfwWindowShouldClose(window.window)) {
		glfwPollEvents();
		render.DrawFrame(buffer.GetIndexBuffer(),
			buffer.GetVertexBuffer(),
			device,
			mesh,
			depth,
			window,
			pipeline,
			swapChain,
			descriptor);
	}
	device.WaitIdle();
}

void Vulkan::InitVulkan() {
	instance.CreateInstance(validation);
	validation.SetupMessenger(instance.GetVkInstance());
	window.CreateSurface(instance.GetVkInstance());

	device.PickPhysicalDevice(instance.GetVkInstance(), window.GetSurface());
	device.CreateLogicalDevice(window.GetSurface(), validation);

	swapChain.Create(device, window);
	swapChain.CreateImageViews(device.GetLogical());

	pipeline.CreateRenderPass(swapChain.GetImageFormat(), device);
	descriptor.CreateDescriptorSetLayout(device.GetLogical());

	pipeline.CreateGraphicsPipeline(shader,
		device.GetLogical(),
		descriptor.GetSetLayout());

	render.CreateCommandPool(device, window.GetSurface());
	depth.CreateDepthResources(device, swapChain.GetExtent());
	swapChain.CreateFrameBuffers(device.GetLogical(),
		depth,
		pipeline.GetRenderPass());

	texture.CreateTextureImage(device, mesh, render);
	texture.CreateTextureImageView(device.GetLogical());
	texture.CreateTextureSampler(device);

	buffer.CreateVertexBuffer(device, mesh, render);
	buffer.CreateIndexBuffer(device, mesh, render);

	descriptor.CreateUniformBuffers(device);
	descriptor.CreateDescriptorPool(device.GetLogical());
	descriptor.CreateDescriptorSets(device.GetLogical(), texture);

	render.CreateCommandBuffers(device.GetLogical(),
		descriptor.GetUniformBuffer().GetMaxFramesInFlight());
	render.CreateSyncObjects(device.GetLogical(),
		descriptor.GetUniformBuffer().GetMaxFramesInFlight());
}

void Vulkan::Cleanup() const {
	swapChain.CleanupSwapChain(device.GetLogical(), depth);
	pipeline.DestroyGraphicsPipeline(device.GetLogical());

	descriptor.Destroy(device.GetLogical());
	texture.Destroy(device.GetLogical());

	buffer.CleanupBuffers(device.GetLogical());
	render.DestroySyncObjects(device.GetLogical(),
		descriptor.GetUniformBuffer().GetMaxFramesInFlight());
	render.DestroyCommandPool(device.GetLogical());

	device.DestroyLogicalDevice();

	validation.DestroyMessenger(instance.GetVkInstance());
	window.DestroySurface(instance.GetVkInstance());
	instance.DestroyInstance();
	window.DestroyWindow();
}
