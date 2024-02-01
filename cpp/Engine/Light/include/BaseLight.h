#pragma once

#include <Engine/Scene/include/SceneObject.h>

class BaseLight : public SceneObject {
 protected:
  LightType type = LightType::Unset;
  float intensity = 1;
  glm::vec4 color = Vec4One;

 public:
  template <typename... Args>
  explicit BaseLight(Args&&... args)
      : SceneObject(std::forward<Args>(args)...) {}
  ~BaseLight() override = default;

  virtual void OnCreate() override;

  virtual LightType& GetType() { return type; }
  virtual float& GetIntensity() { return intensity; }
  virtual glm::vec4& GetColor() { return color; }

  virtual glm::vec3& GetPosition() { return GetAbsolutePosition(); }
  virtual glm::vec3& GetNormal() { return GetAbsoluteForward(); }
};