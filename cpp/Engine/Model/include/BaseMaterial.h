#pragma once

#include <Engine/System/include/BaseObject.h>
#include <Engine/Utility/include/MathUtils.h>
#include <Engine/Utility/include/TypeUtils.h>
#include <assimp/scene.h>
#include <assimp/types.h>

class BaseMaterial final : public BaseObject {
 protected:
  MaterialData params;
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
      params.color = MathUtils::AiColor4D2GlmVec4(color);
      matData->Get(AI_MATKEY_ROUGHNESS_FACTOR, params.roughness);
      matData->Get(AI_MATKEY_METALLIC_FACTOR, params.metallic);
    }
    JsonUtils::ParseMaterialParams(GetRoot() + GetFile(), params);
  }

  std::vector<std::string>& GetShaders() { return shaders; }
  MaterialData& GetParams() { return params; }
};
