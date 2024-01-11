#pragma once

#include "BaseObject.h"

struct MeshData;

class GraphicsInterface : public BaseObject {
 public:
  template <typename... Args>
  explicit GraphicsInterface(const Args&... args) : BaseObject(args...) {}
  ~GraphicsInterface() override = default;

  virtual void CreateWindow(const std::string& title) = 0;
  virtual void InitGraphics() = 0;
  virtual void RendererLoop() = 0;
  virtual void CleanupGraphics() = 0;

  virtual void ParseMeshDatas(std::vector<MeshData*>& meshDatas) = 0;
  virtual float GetViewportAspect() = 0;
};
