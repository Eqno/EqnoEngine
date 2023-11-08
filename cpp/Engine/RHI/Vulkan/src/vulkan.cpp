#include "../include/vulkan.h"

#include <ranges>

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
	swapChain.CleanupSwapChain(device.GetLogical(), depth);
	render.Destroy(device.GetLogical());

	device.DestroyLogicalDevice();
	validation.DestroyMessenger(instance.GetVkInstance());

	window.DestroySurface(instance.GetVkInstance());
	instance.DestroyInstance();
	window.DestroyWindow();
}

void Vulkan::InitStartScene() {
	// scene = new BaseScene(game + "Assets/Scenes/Launcher");
	// for (const std::pair<std::string, std::vector<BaseObject*>> objects:
	//      BaseObject::BaseObjects) {
	// 	for (BaseObject* object: objects.second) {
	// 		object->OnCreate();
	// 	}
	// }
	// for (const std::pair<std::string, std::vector<BaseObject*>> objects:
	//      BaseObject::BaseObjects) {
	// 	for (BaseObject* object: objects.second) {
	// 		object->OnStart();
	// 	}
	// }
}

void Vulkan::CleanupStartScene() const {
	// for (const std::pair<std::string, std::vector<BaseObject*>> objects:
	//      BaseObject::BaseObjects) {
	// 	for (BaseObject* object: objects.second) {
	// 		object->OnStop();
	// 	}
	// }
	// for (const std::pair<std::string, std::vector<BaseObject*>> objects:
	//      BaseObject::BaseObjects) {
	// 	for (BaseObject* object: objects.second) {
	// 		object->OnDestroy();
	// 	}
	// }
	// delete scene;
}
