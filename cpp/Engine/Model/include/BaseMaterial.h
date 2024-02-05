#pragma once

#include <Engine/System/include/BaseObject.h>
#include <assimp/scene.h>
#include <assimp/types.h>

class BaseScene;

class BaseMaterial final : public BaseObject {
 protected:
  std::string name;
  glm::vec4 color;
  float roughness;
  float metallic;

  std::weak_ptr<BaseScene> scene;
  std::vector<std::string> shaders;
  aiMaterial* matData = nullptr;

 public:
  template <typename... Args>
  explicit BaseMaterial(aiMaterial* matData, const std::string& name,
                        Args&&... args)
      : matData(matData), name(name), BaseObject(std::forward<Args>(args)...) {}
  ~BaseMaterial() override = default;

  virtual void OnCreate() override;
  virtual void OnDestroy() override;

  std::string& GetName() { return name; }
  glm::vec4& GetColor() { return color; }
  float& GetRoughness() { return roughness; }
  float& GetMetallic() { return metallic; }
  std::vector<std::string>& GetShaders() { return shaders; }
};