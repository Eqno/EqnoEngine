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
  // Members
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
  void ReleaseBufferLocks();

  void UpdateGameDeltaTime();
  void UpdateRenderDeltaTime();

  void ShowGameFrameCount();
  void ShowRenderFrameCount();

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

 private:
  const float oneSecondTime = 1;

  // Config
  bool showRenderFrameCount = false;
  bool showGameFrameCount = false;

  bool enableMipmap = false;
  bool enableZPrePass = false;
  bool enableShadowMap = false;
  bool enableDeferred = false;
  bool enableShaderDebug = false;

  int shadowMapWidth = -1;
  int shadowMapHeight = -1;

  int msaaSamples = 4;
  float depthBiasClamp = 0;
  float depthBiasSlopeFactor = 2.5f;
  float depthBiasConstantFactor = 1.5f;

  void InitConfig();

 public:
  bool GetEnableMipmap() const { return enableMipmap; }
  bool GetEnableZPrePass() const { return enableZPrePass; }
  bool GetEnableShadowMap() const { return enableShadowMap; }
  bool GetEnableDeferred() const { return enableDeferred; }
  bool GetEnableShaderDebug() const { return enableShaderDebug; }

  uint32_t GetShadowMapWidth() const {
    return shadowMapWidth >= 0 ? shadowMapWidth
                               : render.GetSwapChainExtentWidth();
  }
  uint32_t GetShadowMapHeight() const {
    return shadowMapHeight >= 0 ? shadowMapHeight
                                : render.GetSwapChainExtentHeight();
  }
  int GetMSAASamples() const { return msaaSamples; }
  float GetDepthBiasClamp() const { return depthBiasClamp; }
  float GetDepthBiasSlopeFactor() const { return depthBiasSlopeFactor; }
  float GetDepthBiasConstantFactor() const { return depthBiasConstantFactor; }
  std::weak_ptr<LightChannel> GetLightChannelByName(const std::string& name);
};
