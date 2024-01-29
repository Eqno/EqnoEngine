#pragma once

#include <Engine/Scene/include/SceneObject.h>

class BaseLight : public SceneObject {
 protected:
  float intensity = 1;

 public:
  static std::unordered_map<std::string, BaseLight*> BaseLights;

  template <typename... Args>
  explicit BaseLight(Args&&... args)
      : SceneObject(std::forward<Args>(args)...) {}
  ~BaseLight() override = default;

  virtual void OnCreate() override {
    SceneObject::OnCreate();
    BaseLights[name] = this;
    intensity = JSON_CONFIG(Float, "Intensity");
  }
};