#pragma once

#include "buffer.h"
#include "depth.h"
#include "device.h"
#include "instance.h"
#include "mesh.h"
#include "pipeline.h"
#include "render.h"
#include "shader.h"
#include "swapchain.h"
#include "texture.h"
#include "uniform.h"
#include "validation.h"
#include "window.h"

class Vulkan {
	Mesh mesh;
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
	void Run();
	void MainLoop();

private:
	void InitVulkan();
	void Cleanup() const;
};
