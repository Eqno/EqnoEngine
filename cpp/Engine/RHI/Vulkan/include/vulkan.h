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

class Vulkan final : public GraphicsInterface, public Base {
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

  Draw* GetDrawByShader(const std::string& shaderPath);
  void ParseMeshDatas(std::vector<MeshData*>& meshDatas) override;
  float GetViewportAspect() override;

  virtual void OnCreate() override {
    RegisterMember(depth, device, window, render, instance, swapChain,
                   validation);
  }
  virtual void OnStart() override {}
  virtual void OnUpdate() override {}
  virtual void OnStop() override {}
  virtual void OnDestroy() override {}
};
