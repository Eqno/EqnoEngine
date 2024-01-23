#pragma once

#include <assimp/scene.h>

#include "Engine/System/include/BaseObject.h"
#include "Engine/Utility/include/TypeUtils.h"

class BaseMaterial final : public BaseObject {
  MaterialData data;

 public:
  template <typename... Args>
  explicit BaseMaterial(Args&&... args)
      : BaseObject(std::forward<Args>(args)...) {}
  ~BaseMaterial() override = default;

  virtual void OnCreate() override {
    data.shader = JSON_CONFIG(String, "Shader");
    JsonUtils::ParseMaterialParams(GetRoot() + GetFile(), data);
  }

  const std::string& GetShader() const { return data.shader; }
  const const MaterialData& GetData() { return data; }

  const glm::vec4& GetColor() const { return data.color; }
  const float GetRoughness() const { return data.roughness; }
  const float GetMetallic() const { return data.metallic; }
};
