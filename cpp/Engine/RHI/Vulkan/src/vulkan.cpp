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
		render.DrawFrame(device, draws, depth, window, swapChain);
	}
	device.WaitIdle();
}

void Vulkan::InitVulkan() {
	instance.CreateInstance(validation);
	validation.SetupMessenger(instance.GetVkInstance());
	window.CreateSurface(instance.GetVkInstance());

	device.PickPhysicalDevice(instance.GetVkInstance(), window.GetSurface());
	device.CreateLogicalDevice(window.GetSurface(), validation);

	swapChain.Create("NOSRGB", "NOSRGB", device, window);
	swapChain.CreateImageViews(device.GetLogical());
	render.CreateRenderPass(swapChain.GetImageFormat(), device);
	draws.emplace_back(device, Config::SHADER_PATH, render.GetRenderPass());

	render.CreateCommandPool(device, window.GetSurface());
	render.CreateUniformBuffers(device);

	depth.CreateDepthResources(device, swapChain.GetExtent());
	swapChain.CreateFrameBuffers(device.GetLogical(),
		depth,
		render.GetRenderPass());
	
	draws[0].Load(device,
		render,
		{{Config::MODEL_PATH, {Config::TEXTURE_PATH}}});

	render.CreateCommandBuffers(device.GetLogical());
	render.CreateSyncObjects(device.GetLogical());
}

void Vulkan::Cleanup() const {
	swapChain.CleanupSwapChain(device.GetLogical(), depth);
	for (const Draw& draw: draws) {
		draw.Destroy(device.GetLogical());
	}
	render.Destroy(device.GetLogical());
	device.DestroyLogicalDevice();
	validation.DestroyMessenger(instance.GetVkInstance());
	window.DestroySurface(instance.GetVkInstance());
	instance.DestroyInstance();
	window.DestroyWindow();
}
