#pragma once

#include "Engine/Model/include/BaseMaterial.h"
#include "Engine/System/include/BaseObject.h"
#include "Engine/System/include/GraphicsInterface.h"
#include "SceneObject.h"

class BaseScene final : public BaseObject {
  SceneObject* rootObject = nullptr;
  GraphicsInterface* graphics = nullptr;
  std::unordered_map<std::string, BaseMaterial*> materials;

 public:
  BaseMaterial* GetMaterialByPath(const std::string& path) {
    if (!materials.contains(path)) {
      materials[path] = Create<BaseMaterial>(GetRoot(), path);
    }
    return materials[path];
  }
  template <typename... Args>
  explicit BaseScene(GraphicsInterface* graphics, Args&&... args)
      : graphics(graphics), BaseObject(std::forward<Args>(args)...) {}
  ~BaseScene() override = default;

  [[nodiscard]] GraphicsInterface* GetGraphics() const { return graphics; }

  virtual void OnCreate() override;
  virtual void OnStart() override { BaseObject::OnStart(); }
  virtual void OnUpdate() override { BaseObject::OnUpdate(); }
  virtual void OnStop() override { BaseObject::OnStop(); }
  virtual void OnDestroy() override;
};
