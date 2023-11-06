#pragma once

#include "depth.h"
#include "device.h"
#include "instance.h"
#include "render.h"
#include "swapchain.h"
#include "validation.h"
#include "window.h"

#include "Engine/System/include/base.h"
#include "Engine/Scene/include/scene.h"
#include "Engine/Model/include/model.h"

class Vulkan {
	Depth depth;
	Device device;
	Window window;
	Render render;
	Instance instance;
	SwapChain swapChain;
	Validation validation;

	Scene* scene = nullptr;
	std::string game = "Unset";

public:
	explicit Vulkan(const std::string& gameName) {
		game = gameName;
	}

	void Run();
	void MainLoop();

private:
	void InitVulkan();
	void Cleanup() const;
};
