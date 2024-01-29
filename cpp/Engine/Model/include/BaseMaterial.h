#pragma once

#include <assimp/scene.h>

#include "Engine/System/include/BaseObject.h"
#include "Engine/Utility/include/TypeUtils.h"

class BaseMaterial final : public BaseObject {
 protected:
  std::string shader;
  glm::vec4 color;
  float roughness;
  float metallic;

 public:
  template <typename... Args>
  explicit BaseMaterial(Args&&... args)
      : BaseObject(std::forward<Args>(args)...) {}
  ~BaseMaterial() override = default;

  virtual void OnCreate() override {
    BaseObject::OnCreate();

    shader = JSON_CONFIG(String, "Shader");
    JsonUtils::ParseMaterialParams(GetRoot() + GetFile(), color, roughness,
                                   metallic);
  }

  std::string& GetShader() { return shader; }
  glm::vec4& GetColor() { return color; }
  float& GetRoughness() { return roughness; }
  float& GetMetallic() { return metallic; }
};
