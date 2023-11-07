#include "../include/Application.h"

#include "Engine/RHI/Vulkan/include/vulkan.h"

void Application::CreateGraphics() {
	const std::string apiPath = JsonUtils::ReadStringFromFile(
		gameRoot + "Index", "GraphicsConfig");

	if (const std::string rhiType = JsonUtils::ReadStringFromFile(
		gameRoot + apiPath, "RenderHardwareInterface"); rhiType == "Vulkan") {
		// graphics = new Vulkan;
	}
	else if (rhiType == "DirectX") {
		throw std::runtime_error("DirectX not supported now!");
	}
	else {
		throw std::runtime_error(rhiType + " not supported now!");
	}

	// graphics->CreateWindow();
	// graphics->InitGraphics();
	// graphics->RendererLoop();
	// graphics->CleanupGraphics();
}
