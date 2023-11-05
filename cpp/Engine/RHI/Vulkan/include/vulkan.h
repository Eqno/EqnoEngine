#pragma once

#include "depth.h"
#include "device.h"
#include "draw.h"
#include "instance.h"
#include "render.h"
#include "swapchain.h"
#include "validation.h"
#include "window.h"

class Vulkan {
	Depth depth;
	Device device;
	Window window;
	Render render;
	Instance instance;
	SwapChain swapChain;
	Validation validation;

	std::vector<Draw> draws;

public:
	void Run();
	void MainLoop();

private:
	void InitVulkan();
	void Cleanup() const;
};
