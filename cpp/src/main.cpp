#include <iostream>

#include "buffer.h"
#include "device.h"
#include "instance.h"
#include "pipeline.h"
#include "render.h"
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
	Render render;
	Instance instance;
	Pipeline pipeline;
	SwapChain swapChain;
	Descriptor descriptor;
	Validation validation;

public:
	void Run() {
		window.CreateWindow();
		InitVulkan();
		MainLoop();
		Cleanup();
	}

	void MainLoop() {
		while (!glfwWindowShouldClose(window.window)) {
			glfwPollEvents();
			render.DrawFrame(
				buffer.GetIndexBuffer(),
				buffer.GetVertexBuffer(),
				device,
				window,
				pipeline,
				swapChain,
				descriptor
			);
		}
		device.WaitIdle();
	}

private:
	void InitVulkan() {
		instance.CreateInstance(validation);
		validation.SetupMessenger(instance.GetVkInstance());
		window.CreateSurface(instance.GetVkInstance());

		device.PickPhysicalDevice(instance.GetVkInstance(), window.GetSurface());
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

		render.CreateCommandPool(device, window.GetSurface());
		buffer.CreateVertexBuffer(device, render);
		buffer.CreateIndexBuffer(device, render);

		descriptor.CreateUniformBuffers(device);
		descriptor.CreateDescriptorPool(device.GetLogical());
		descriptor.CreateDescriptorSets(device.GetLogical());

		render.CreateCommandBuffers(device.GetLogical());
		render.CreateSyncObjects(device.GetLogical());
	}

	void Cleanup() const {
		swapChain.CleanupSwapChain(device.GetLogical());
		pipeline.DestroyGraphicsPipeline(device.GetLogical());
		descriptor.Destroy(device.GetLogical());

		buffer.CleanupBuffers(device.GetLogical());
		render.DestroySyncObjects(device.GetLogical());
		render.DestroyCommandPool(device.GetLogical());

		device.DestroyLogicalDevice();

		validation.DestroyMessenger(instance.GetVkInstance());
		window.DestroySurface(instance.GetVkInstance());
		instance.DestroyInstance();

		window.DestroyWindow();
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
