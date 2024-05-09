#pragma once

#include <Engine/System/include/BaseObject.h>
#include <GLFW/glfw3.h>

#include <atomic>
#include <mutex>

struct MeshData;

class GraphicsInterface : public BaseObject {
  bool renderLoopEnd = false;
  bool renderLoopShouldEnd = false;

 protected:
  int msaaSamples = 4;
  bool enableMipmap = false;
  bool enableZPrePass = false;
  bool enableShadowMap = false;
  bool enableDeferred = false;
  bool enableShaderDebug = false;

  bool showRenderFrameCount = false;
  bool showGameFrameCount = false;

  uint32_t renderFrameCount = 0;
  uint32_t gameFrameCount = 0;

  int shadowMapWidth = -1;
  int shadowMapHeight = -1;

  float depthBiasClamp = 0;
  float depthBiasSlopeFactor = 2.5f;
  float depthBiasConstantFactor = 1.5f;

 public:
  template <typename... Args>
  explicit GraphicsInterface(Args&&... args)
      : BaseObject(std::forward<Args>(args)...) {}
  ~GraphicsInterface() override = default;

  virtual void CreateWindow(const std::string& title) = 0;
  virtual void InitGraphics() = 0;
  virtual void CleanupGraphics() = 0;

  virtual void GameLoop() = 0;
  virtual void RenderLoop() = 0;

  virtual void ParseMeshData() = 0;
  virtual void ParseMeshData(std::weak_ptr<MeshData> meshData) = 0;

  virtual GLFWwindow* GetWindow() const = 0;
  virtual float GetViewportAspect() = 0;

  virtual int GetWindowWidth() = 0;
  virtual int GetWindowHeight() = 0;

  virtual bool GetRenderLoopEnd() { return renderLoopEnd; }
  virtual void SetRenderLoopEnd(bool inEnd) { renderLoopEnd = inEnd; }

  virtual bool GetRenderLoopShouldEnd() { return renderLoopShouldEnd; }
  virtual void SetRenderLoopShouldEnd(bool inEnd) {
    renderLoopShouldEnd = inEnd;
  }

  virtual int GetMSAASamples() const { return msaaSamples; }
  virtual bool GetEnableMipmap() const { return enableMipmap; }
  virtual bool GetEnableZPrePass() const { return enableZPrePass; }
  virtual bool GetEnableShadowMap() const { return enableShadowMap; }
  virtual bool GetEnableDeferred() const { return enableDeferred; }
  virtual bool GetEnableShaderDebug() const { return enableShaderDebug; }

  virtual float GetDepthBiasClamp() const { return depthBiasClamp; }
  virtual float GetDepthBiasSlopeFactor() const { return depthBiasSlopeFactor; }
  virtual float GetDepthBiasConstantFactor() const {
    return depthBiasConstantFactor;
  }

  virtual bool& GetShowRenderFrame() { return showRenderFrameCount; }
  virtual bool& GetShowGameFrame() { return showGameFrameCount; }

  virtual uint32_t GetRenderFrameCount() const { return renderFrameCount; }
  virtual uint32_t GetGameFrameCount() const { return gameFrameCount; }
};