#pragma once

#include <Engine/System/include/BaseObject.h>

#include <atomic>
#include <mutex>

struct MeshData;

class GraphicsInterface : public BaseObject {
  bool renderLoopEnd = false;

 public:
  template <typename... Args>
  explicit GraphicsInterface(Args&&... args)
      : BaseObject(std::forward<Args>(args)...) {}
  ~GraphicsInterface() override = default;

  std::mutex updateMeshDataMutex;

  virtual void CreateWindow(const std::string& title) = 0;
  virtual void InitGraphics() = 0;
  virtual void CleanupGraphics() = 0;

  virtual void GameLoop() = 0;
  virtual void RenderLoop() = 0;

  virtual void ParseMeshDatas() = 0;
  virtual void ParseMeshDatas(
      std::vector<std::weak_ptr<MeshData>>&& meshDatas) = 0;
  virtual float GetViewportAspect() = 0;
  virtual bool GetRenderLoopEnd() { return renderLoopEnd; }
  virtual void SetRenderLoopEnd(bool inEnd) { renderLoopEnd = inEnd; }
};
