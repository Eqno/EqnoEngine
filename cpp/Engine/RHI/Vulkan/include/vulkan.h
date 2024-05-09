#pragma once

#include <Engine/System/include/GraphicsInterface.h>
#include <GLFW/glfw3.h>

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
  Render render;
  VkWindow window;
  Instance instance;
  Validation validation;

  int windowWidth = 1280;
  int windowHeight = 720;

  BufferManager bufferManager;
  Application* appPointer = nullptr;
  std::unordered_map<std::string, Draw*> drawsByShader;
  std::unordered_map<int, Draw*> drawsByPipeline;

  std::mutex updateMeshDataMutex;
  std::atomic<bool> needToUpdateMeshDatas = false;
  std::queue<std::weak_ptr<MeshData>> meshDataQueue;

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

  int GetWindowWidth() override { return windowWidth; }
  int GetWindowHeight() override { return windowHeight; }

  BufferManager& GetBufferManager() { return bufferManager; }
  float GetViewportAspect() override;

  void ParseMeshData() override;
  void ParseMeshData(std::weak_ptr<MeshData> meshData) override;

  virtual void OnCreate() override {
    GraphicsInterface::OnCreate();
    RegisterMember(device, window, render, instance, validation);
  }
  virtual void OnStart() override { GraphicsInterface::OnStart(); }
  virtual void OnUpdate() override { GraphicsInterface::OnUpdate(); }
  virtual void OnStop() override { GraphicsInterface::OnStop(); }
  virtual void OnDestroy() override { GraphicsInterface::OnDestroy(); }

  void RemoveDrawByShader(const std::string& shader) {
    drawsByShader.erase(shader);
  }
  void RemoveDrawByPipeline(const int id) { drawsByPipeline.erase(id); }
  GLFWwindow* GetWindow() const override { return window.GetWindow(); }

 private:
  const float oneSecondTime = 1;

  // Config
  void InitConfig();

 public:
  std::weak_ptr<LightChannel> GetLightChannelByName(const std::string& name);

  virtual bool GetEnableEditor();
  virtual GLFWwindow* GetParentWindow();
  virtual GLFWwindow* GetEditorWindow();
  virtual bool GetLaunchSceneInEditor();

  uint32_t GetShadowMapWidth() const {
    return shadowMapWidth >= 0 ? shadowMapWidth
                               : render.GetSwapChainExtentWidth();
  }
  uint32_t GetShadowMapHeight() const {
    return shadowMapHeight >= 0 ? shadowMapHeight
                                : render.GetSwapChainExtentHeight();
  }
};
