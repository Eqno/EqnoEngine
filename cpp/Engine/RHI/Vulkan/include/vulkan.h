#pragma once

#include <Engine/System/include/GraphicsInterface.h>

#include "depth.h"
#include "device.h"
#include "draw.h"
#include "instance.h"
#include "render.h"
#include "swapchain.h"
#include "validation.h"
#include "window.h"

class Vulkan final : public GraphicsInterface {
  Depth depth;
  Device device;
  Window window;
  Render render;
  Instance instance;
  SwapChain swapChain;
  Validation validation;
  std::unordered_map<std::string, Draw*> draws;

 public:
  template <typename... Args>
  explicit Vulkan(const Args&... args) : GraphicsInterface(args...) {}
  ~Vulkan() override = default;

  void CreateWindow(const std::string& title) override;
  void InitGraphics() override;
  void RendererLoop() override;
  void CleanupGraphics() override;

  Draw* GetDrawByShader(const std::string& shaderPath) {
    if (!draws.contains(shaderPath)) {
      draws[shaderPath] = new Draw(device, shaderPath, render.GetRenderPass());
    }
    return draws[shaderPath];
  }

  void ParseMeshDatas(std::vector<MeshData*>& meshDatas) override;
  float GetViewportAspect() override;

  void OnCreate() override {}
  void OnStart() override {}
  void OnUpdate() override {}
  void OnStop() override {}
  void OnDestroy() override {}
};
