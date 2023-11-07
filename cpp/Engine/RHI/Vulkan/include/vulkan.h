#pragma once

#include "depth.h"
#include "device.h"
#include "instance.h"
#include "render.h"
#include "swapchain.h"
#include "validation.h"
#include "window.h"

#include "Engine/System/include/GraphicsInterface.h"

class Vulkan final: public GraphicsInterface {
	Depth depth;
	Device device;
	Window window;
	Render render;
	Instance instance;
	SwapChain swapChain;
	Validation validation;

	void InitVulkan();
	void CleanupVulkan() const;
	void InitStartScene();
	void CleanupStartScene() const;

public:
	Vulkan() = delete;

	void CreateWindow() override;
	void InitGraphics() override;
	void RendererLoop() override;
	void CleanupGraphics() override;
};
