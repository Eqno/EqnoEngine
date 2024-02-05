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

  BufferManager bufferManager;
  std::unordered_map<std::string, Draw*> draws;

 public:
  template <typename... Args>
  explicit Vulkan(const Args&... args) : GraphicsInterface(args...) {}
  ~Vulkan() override = default;

  void CreateWindow(const std::string& title) override;
  void InitGraphics() override;
  void TriggerOnUpdate() override;
  void RendererLoop() override;
  void CleanupGraphics() override;

  BufferManager& GetBufferManager() { return bufferManager; }
  void ParseMeshDatas(std::vector<MeshData*>& meshDatas) override;
  float GetViewportAspect() override;

  virtual void OnCreate() override {
    GraphicsInterface::OnCreate();
    RegisterMember(depth, device, window, render, instance, swapChain,
                   validation);
  }
  virtual void OnStart() override { GraphicsInterface::OnStart(); }
  virtual void OnUpdate() override { GraphicsInterface::OnUpdate(); }
  virtual void OnStop() override { GraphicsInterface::OnStop(); }
  virtual void OnDestroy() override { GraphicsInterface::OnDestroy(); }
};
