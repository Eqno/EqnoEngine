#pragma once

#include <Engine/System/include/BaseObject.h>
#include <GLFW/glfw3.h>

#include <atomic>
#include <mutex>

struct MeshData;

class GraphicsInterface : public BaseObject {
  bool renderLoopEnd = false;
  bool renderLoopShouldEnd = false;

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
};
