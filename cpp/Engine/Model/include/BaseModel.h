#pragma once

#include "Engine/Scene/include/BaseScene.h"
#include "Engine/Scene/include/SceneObject.h"
#include "Engine/System/include/BaseObject.h"
#include "Engine/Utility/include/TypeUtils.h"

class BaseModel final : public SceneObject {
  std::vector<MeshData*> meshes;
  GraphicsInterface* graphics;

  void LoadFbxDatas(const std::string& fbxPath, const unsigned int parserFlags);

 public:
  template <typename... Args>
  explicit BaseModel(GraphicsInterface* graphics, Args&&... args)
      : graphics(graphics), SceneObject(std::forward<Args>(args)...) {}
  ~BaseModel() override = default;

  virtual void OnCreate() override;
  virtual void OnUpdate() override;
  virtual void OnDestroy() override;
  virtual void OnStart() override {}
  virtual void OnStop() override {}
};
