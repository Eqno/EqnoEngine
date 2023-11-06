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
		for (const std::pair<std::string, std::vector<Base*>> objects:
		     Base::Objects) {
			for (Base* object: objects.second) {
				object->OnUpdate();
			}
		}
		render.DrawFrame(device, scene->GetDraws(), depth, window, swapChain);
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

	render.CreateRenderPass(swapChain.GetImageFormat(), device);
	render.CreateCommandPool(device, window.GetSurface());
	render.CreateUniformBuffers(device);

	depth.CreateDepthResources(device, swapChain.GetExtent());
	swapChain.CreateFrameBuffers(device.GetLogical(),
		depth,
		render.GetRenderPass());

	render.CreateCommandBuffers(device.GetLogical());
	render.CreateSyncObjects(device.GetLogical());

	scene = new Scene(game + "Assets/Scenes/Launcher");
	for (const std::pair<std::string, std::vector<Base*>> objects:
	     Base::Objects) {
		for (Base* object: objects.second) {
			object->OnCreate();
		}
	}
	for (const std::pair<std::string, std::vector<Base*>> objects:
	     Base::Objects) {
		for (Base* object: objects.second) {
			object->OnStart();
		}
	}
}

void Vulkan::Cleanup() const {
	for (const std::pair<std::string, std::vector<Base*>> objects:
	     Base::Objects) {
		for (Base* object: objects.second) {
			object->OnStop();
		}
	}
	for (const std::pair<std::string, std::vector<Base*>> objects:
	     Base::Objects) {
		for (Base* object: objects.second) {
			object->OnDestroy();
		}
	}
	delete scene;

	swapChain.CleanupSwapChain(device.GetLogical(), depth);
	render.Destroy(device.GetLogical());
	device.DestroyLogicalDevice();
	validation.DestroyMessenger(instance.GetVkInstance());
	window.DestroySurface(instance.GetVkInstance());
	instance.DestroyInstance();
	window.DestroyWindow();
}

void LoadScene() {}
