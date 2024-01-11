#pragma once

#include <assimp/scene.h>

#include "Engine/System/include/BaseObject.h"
#include "Engine/Utility/include/TypeUtils.h"

class BaseMaterial final : public BaseObject {
  MaterialData data;

 public:
  explicit BaseMaterial(BaseObject* owner, const std::string& root,
                        const std::string& file)
      : BaseObject(owner, root, file) {}
  ~BaseMaterial() override = default;

  void OnCreate() override {
    data.shader = JSON_CONFIG(String, "Shader");
    data.params = JsonUtils::ParseMaterialParams(GetRoot() + GetFile());
  }

  const std::string& GetShader() { return data.shader; }

  const std::vector<std::string>& GetParams() { return data.params; }

  std::vector<std::string> ParseFbxMaterials(const aiScene* scene);
};
