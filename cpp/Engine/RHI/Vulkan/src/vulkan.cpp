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
		render.DrawFrame(mesh, device, depth, window, pipeline, swapChain);
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
	pipeline.CreateDescriptorSetLayout(device.GetLogical());
	pipeline.CreateGraphicsPipeline(shader, device.GetLogical());

	render.CreateCommandPool(device, window.GetSurface());
	depth.CreateDepthResources(device, swapChain.GetExtent());
	swapChain.CreateFrameBuffers(device.GetLogical(),
		depth,
		pipeline.GetRenderPass());

	mesh.InitMesh(device, render, pipeline);

	render.CreateCommandBuffers(device.GetLogical(),
		mesh.GetUniformBuffer().GetMaxFramesInFlight());
	render.CreateSyncObjects(device.GetLogical(),
		mesh.GetUniformBuffer().GetMaxFramesInFlight());
}

void Vulkan::Cleanup() const {
	swapChain.CleanupSwapChain(device.GetLogical(), depth);
	pipeline.DestroyGraphicsPipeline(device.GetLogical());

	mesh.DestroyMesh(device.GetLogical());
	render.DestroySyncObjects(device.GetLogical(),
		mesh.GetUniformBuffer().GetMaxFramesInFlight());
	render.DestroyCommandPool(device.GetLogical());

	device.DestroyLogicalDevice();

	validation.DestroyMessenger(instance.GetVkInstance());
	window.DestroySurface(instance.GetVkInstance());
	instance.DestroyInstance();
	window.DestroyWindow();
}
