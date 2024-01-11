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

  explicit BaseScene(BaseObject* owner, const std::string& root,
                     const std::string& file, GraphicsInterface* graphics)
      : BaseObject(owner, root, file), graphics(graphics) {}
  ~BaseScene() override = default;

  [[nodiscard]] GraphicsInterface* GetGraphics() const { return graphics; }

  virtual void OnCreate() override;
  virtual void OnStart() override {}
  virtual void OnUpdate() override {}
  virtual void OnStop() override {}
  virtual void OnDestroy() override;
};
