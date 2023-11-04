#include <iostream>

#include "buffer.h"
#include "depth.h"
#include "device.h"
#include "instance.h"
#include "pipeline.h"
#include "render.h"
#include "shader.h"
#include "swapchain.h"
#include "texture.h"
#include "uniform.h"
#include "validation.h"
#include "window.h"


class HelloTriangleApplication {
	Depth depth;
	Shader shader;
	Device device;
	Window window;
	Buffer buffer;
	Render render;
	Texture texture;
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
			render.DrawFrame(buffer.GetIndexBuffer(),
				buffer.GetVertexBuffer(),
				device,
				depth,
				window,
				pipeline,
				swapChain,
				descriptor);
		}
		device.WaitIdle();
	}

private:
	void InitVulkan() {
		instance.CreateInstance(validation);
		validation.SetupMessenger(instance.GetVkInstance());
		window.CreateSurface(instance.GetVkInstance());

		device.PickPhysicalDevice(instance.GetVkInstance(),
			window.GetSurface());
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

		texture.CreateTextureImage(device, render);
		texture.CreateTextureImageView(device.GetLogical());
		texture.CreateTextureSampler(device);

		buffer.CreateVertexBuffer(device, render);
		buffer.CreateIndexBuffer(device, render);

		descriptor.CreateUniformBuffers(device);
		descriptor.CreateDescriptorPool(device.GetLogical());
		descriptor.CreateDescriptorSets(device.GetLogical(), texture);

		render.CreateCommandBuffers(device.GetLogical(),
			descriptor.GetUniformBuffer().GetMaxFramesInFlight());
		render.CreateSyncObjects(device.GetLogical(),
			descriptor.GetUniformBuffer().GetMaxFramesInFlight());
	}

	void Cleanup() const {
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
