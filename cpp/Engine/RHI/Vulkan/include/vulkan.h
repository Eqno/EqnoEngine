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

	void InitStartScene();
	void CleanupStartScene() const;

public:
	explicit Vulkan(const std::string& name,
		const std::string& root,
		const std::string& file) : GraphicsInterface(name, root, file) {}

	~Vulkan() override = default;

	void CreateWindow(const std::string& title) override;
	void InitGraphics() override;
	void RendererLoop() override;
	void CleanupGraphics() override;
};
