#pragma once

#include "depth.h"
#include "device.h"
#include "instance.h"
#include "render.h"
#include "swapchain.h"
#include "validation.h"
#include "window.h"
#include "draw.h"

#include "Engine/System/include/GraphicsInterface.h"

class Vulkan final: public GraphicsInterface {
	Depth depth;
	Device device;
	Window window;
	Render render;
	Instance instance;
	SwapChain swapChain;
	Validation validation;
	std::unordered_map<std::string, Draw*> draws;

public:
	Draw* GetDrawByShader(const std::string& shaderPath) {
		if (!draws.contains(shaderPath)) {
			draws[shaderPath] = new Draw(device, shaderPath,
				render.GetRenderPass());
		}
		return draws[shaderPath];
	}

	explicit Vulkan(const std::string& root,
		const std::string& file) : GraphicsInterface(root, file) {}

	~Vulkan() override = default;

	void CreateWindow(const std::string& title) override;
	void InitGraphics() override;
	void RendererLoop() override;
	void CleanupGraphics() override;

	void ParseMeshDatas(std::vector<MeshData*>& meshDatas) override;
};
