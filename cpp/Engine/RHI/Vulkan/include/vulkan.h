#pragma once

#include <Engine/System/include/GraphicsInterface.h>

#include <atomic>
#include <mutex>
#include <queue>

#include "depth.h"
#include "device.h"
#include "draw.h"
#include "instance.h"
#include "render.h"
#include "swapchain.h"
#include "validation.h"
#include "window.h"

class Vulkan final : public GraphicsInterface, public Base {
  Device device;
  Window window;
  Render render;
  Instance instance;
  Validation validation;

  BufferManager bufferManager;
  Application* appPointer = nullptr;
  std::unordered_map<std::string, Draw*> draws;

  std::mutex updateMeshDataMutex;
  std::atomic<bool> needToUpdateMeshDatas = false;
  std::queue<std::vector<std::weak_ptr<MeshData>>> meshDataQueue;

 public:
  template <typename... Args>
  explicit Vulkan(const Args&... args) : GraphicsInterface(args...) {}
  ~Vulkan() override = default;

  void CreateWindow(const std::string& title) override;
  void InitGraphics() override;
  void TriggerOnUpdate(
      std::unordered_map<int, std::weak_ptr<BaseLight>>& lightsById);
  void CleanupGraphics() override;

  void GameLoop() override;
  void RenderLoop() override;

  void GetAppPointer();
  void UpdateDeltaTime();
  void ReleaseBufferLocks();

  BufferManager& GetBufferManager() { return bufferManager; }
  void ParseMeshDatas() override;
  void ParseMeshDatas(
      std::vector<std::weak_ptr<MeshData>>&& meshDatas) override;
  float GetViewportAspect() override;

  virtual void OnCreate() override {
    GraphicsInterface::OnCreate();
    RegisterMember(device, window, render, instance, validation);
  }
  virtual void OnStart() override { GraphicsInterface::OnStart(); }
  virtual void OnUpdate() override { GraphicsInterface::OnUpdate(); }
  virtual void OnStop() override { GraphicsInterface::OnStop(); }
  virtual void OnDestroy() override { GraphicsInterface::OnDestroy(); }
};
