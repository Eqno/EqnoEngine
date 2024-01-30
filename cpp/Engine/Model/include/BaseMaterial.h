#pragma once

#include <assimp/scene.h>

#include "Engine/System/include/BaseObject.h"
#include "Engine/Utility/include/TypeUtils.h"

class BaseMaterial final : public BaseObject {
 protected:
  std::string shader;
  MaterialData params;

 public:
  template <typename... Args>
  explicit BaseMaterial(Args&&... args)
      : BaseObject(std::forward<Args>(args)...) {}
  ~BaseMaterial() override = default;

  virtual void OnCreate() override {
    BaseObject::OnCreate();

    shader = JSON_CONFIG(String, "Shader");
    JsonUtils::ParseMaterialParams(GetRoot() + GetFile(), params);
  }

  std::string& GetShader() { return shader; }
  MaterialData& GetParams() { return params; }
};
