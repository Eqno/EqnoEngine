#include "../include/vulkan.h"

#include <Engine/System/include/Application.h>
#include <Engine/Utility/include/JsonUtils.h>

#include <ranges>

void Vulkan::CreateWindow(const std::string& title) {
  int width = JSON_CONFIG(Int, "DefaultWindowWidth");
  int height = JSON_CONFIG(Int, "DefaultWindowHeight");

  width = width == 0 ? VulkanConfig::DEFAULT_WINDOW_WIDTH : width;
  height = height == 0 ? VulkanConfig::DEFAULT_WINDOW_HEIGHT : height;

  window.CreateWindow(width, height, title);
}

void Vulkan::InitGraphics() {
  instance.CreateInstance(validation);
  validation.SetupMessenger(instance.GetVkInstance());
  window.CreateSurface(instance.GetVkInstance());

  device.PickPhysicalDevice(instance.GetVkInstance(), window.GetSurface());
  device.CreateLogicalDevice(window.GetSurface(), validation);

  std::string imageFormat = JSON_CONFIG(String, "SwapChainSurfaceImageFormat");
  std::string colorSpace = JSON_CONFIG(String, "SwapChainSurfaceColorSpace");
  swapChain.CreateRenderTarget(imageFormat, colorSpace, device, window);

  render.CreateRenderPass(swapChain.GetImageFormat(), device);
  render.CreateCommandPool(device, window.GetSurface());

  depth.CreateDepthResources(device, swapChain.GetExtent());
  swapChain.CreateFrameBuffers(device.GetLogical(), depth,
                               render.GetRenderPass());

  render.CreateCommandBuffers(device.GetLogical());
  render.CreateSyncObjects(device.GetLogical());
}

void Vulkan::RendererLoop() {
  while (!glfwWindowShouldClose(window.window)) {
    glfwPollEvents();
    dynamic_cast<Application*>(_owner)->TriggerOnUpdate();
    render.DrawFrame(device, draws, depth, window, swapChain);
  }
  device.WaitIdle();
}

Draw* Vulkan::GetDrawByShader(const std::string& shaderPath) {
  if (!draws.contains(shaderPath)) {
    draws[shaderPath] =
        Base::Create<Draw>(device, shaderPath, render.GetRenderPass());
  }
  return draws[shaderPath];
}

void Vulkan::CleanupGraphics() {
  for (const auto& val : draws | std::views::values) {
    val->DestroyDrawResource(device.GetLogical(), render);
  }

  depth.DestroyDepthResource(device.GetLogical());
  swapChain.CleanupRenderTarget(device.GetLogical());
  render.DestroyRenderResources(device.GetLogical());

  device.DestroyLogicalDevice();
  validation.DestroyMessenger(instance.GetVkInstance());

  window.DestroySurface(instance.GetVkInstance());
  instance.DestroyInstance();
  window.DestroyWindow();
}

void Vulkan::ParseMeshDatas(std::vector<MeshData*>& meshDatas) {
  for (const MeshData* data : meshDatas) {
    Draw* draw = GetDrawByShader(GetRoot() + data->material.shader);
    draw->LoadDrawResource(device, render, data);
  }
}
float Vulkan::GetViewportAspect() { return swapChain.GetViewportAspect(); }