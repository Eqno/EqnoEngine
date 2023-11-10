#include "../include/vulkan.h"

#include <ranges>

#include "Engine/Model/include/BaseModel.h"
#include "Engine/Utility/include/JsonUtils.h"

void Vulkan::CreateWindow(const std::string& title) {
	int width = JSON_CONFIG(Int, "DefaultWindowWidth");
	int height = JSON_CONFIG(Int, "DefaultWindowHeight");
	width = width == 0 ? VulkanConfig::DEFAULT_WINDOW_WIDTH : width;
	height = height == 0 ? VulkanConfig::DEFAULT_WINDOW_HEIGHT : height;
	window.CreateWindow(width, height, title);
}

void Vulkan::InitGraphics() {
	instance.CreateInstance(validation);
	validation.SetupMessenger(instance.GetVkInstance());
	window.CreateSurface(instance.GetVkInstance());

	device.PickPhysicalDevice(instance.GetVkInstance(), window.GetSurface());
	device.CreateLogicalDevice(window.GetSurface(), validation);
	swapChain.Create(JSON_CONFIG(String, "SwapChainSurfaceImageFormat"),
		JSON_CONFIG(String, "SwapChainSurfaceColorSpace"), device, window);

	render.CreateRenderPass(swapChain.GetImageFormat(), device);
	render.CreateCommandPool(device, window.GetSurface());
	render.CreateUniformBuffers(device);

	depth.CreateDepthResources(device, swapChain.GetExtent());
	swapChain.CreateFrameBuffers(device.GetLogical(), depth,
		render.GetRenderPass());

	render.CreateCommandBuffers(device.GetLogical());
	render.CreateSyncObjects(device.GetLogical());
}

void Vulkan::RendererLoop() {
	while (!glfwWindowShouldClose(window.window)) {
		glfwPollEvents();
		for (const auto& val: BaseObjects | std::views::values) {
			for (BaseObject* object: val) {
				object->OnUpdate();
			}
		}
		render.DrawFrame(device, draws, depth, window, swapChain);
	}
	device.WaitIdle();
}

void Vulkan::CleanupGraphics() {
	for (const auto& val: draws | std::views::values) {
		val.Destroy(device.GetLogical());
	}

	swapChain.CleanupSwapChain(device.GetLogical(), depth);
	render.Destroy(device.GetLogical());

	device.DestroyLogicalDevice();
	validation.DestroyMessenger(instance.GetVkInstance());

	window.DestroySurface(instance.GetVkInstance());
	instance.DestroyInstance();
	window.DestroyWindow();
}

void Vulkan::ParseMeshDatas(std::vector<MeshData*>& meshDatas) {
	for (const MeshData* data: meshDatas) {
		if (!draws.contains(data->material)) {
			draws[data->material] = Draw(device, VulkanConfig::SHADER_PATH,
				render.GetRenderPass());
		}
		draws[data->material].Load(device, render, data);
	}
}
