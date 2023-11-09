#include "../include/Application.h"

#include "Engine/RHI/Vulkan/include/vulkan.h"
#include "Engine/Scene/include/StartScene.h"

void Application::CreateGraphics() {
	const std::string apiPath = JSON_CONFIG(String, "GraphicsConfig");
	if (const std::string rhiType = JsonUtils::ReadStringFromFile(
		GetRoot() + apiPath, "RenderHardwareInterface"); rhiType == "Vulkan") {
		graphics = new Vulkan(GetRoot(), apiPath);
	}
	else if (rhiType == "DirectX") {
		throw std::runtime_error("DirectX not supported now!");
	}
	else {
		throw std::runtime_error(rhiType + " not supported now!");
	}
}

void Application::CreateLauncherScene() {
	const std::string scenePath = JSON_CONFIG(String, "LauncherScene");
	scene = new BaseScene(GetRoot(), scenePath, graphics);
}

void Application::Run() const {
	graphics->CreateWindow(JSON_CONFIG(String, "ApplicationName"));
	graphics->InitGraphics();
	graphics->RendererLoop();
	graphics->CleanupGraphics();
}
