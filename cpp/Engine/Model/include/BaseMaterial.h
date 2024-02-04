#pragma once

#include <Engine/System/include/BaseObject.h>
#include <Engine/Utility/include/MathUtils.h>
#include <Engine/Utility/include/TypeUtils.h>
#include <assimp/scene.h>
#include <assimp/types.h>

class BaseMaterial final : public BaseObject {
 protected:
  glm::vec4 color;
  float roughness;
  float metallic;

  std::vector<std::string> shaders;
  aiMaterial* matData = nullptr;

 public:
  template <typename... Args>
  explicit BaseMaterial(aiMaterial* matData, Args&&... args)
      : matData(matData), BaseObject(std::forward<Args>(args)...) {}
  ~BaseMaterial() override = default;

  virtual void OnCreate() override {
    BaseObject::OnCreate();

    JsonUtils::ParseMaterialShaders(GetRoot() + GetFile(), shaders);
    if (matData != nullptr) {
      aiColor4D color;
      matData->Get(AI_MATKEY_COLOR_DIFFUSE, color);
      this->color = MathUtils::AiColor4D2GlmVec4(color);
      matData->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
      matData->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
    }
    JsonUtils::ParseMaterialParams(GetRoot() + GetFile(), color, roughness,
                                   metallic);
  }

  std::vector<std::string>& GetShaders() { return shaders; }
  glm::vec4& GetColor() { return color; }
  float& GetRoughness() { return roughness; }
  float& GetMetallic() { return metallic; }
};
