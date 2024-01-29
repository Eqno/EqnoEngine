#pragma once

#include <Engine/Scene/include/SceneObject.h>

class BaseLight : public SceneObject {
 protected:
  LightType type = LightType::Unset;

  float intensity = 1;
  glm::vec4 color = glm::vec4(1, 1, 1, 1);
  glm::vec3 normal = glm::vec3(0, 0, 0);

 public:
  template <typename... Args>
  explicit BaseLight(Args&&... args)
      : SceneObject(std::forward<Args>(args)...) {}
  ~BaseLight() override = default;

  virtual void OnCreate() override;
  virtual LightType& GetType() { return type; }
  virtual float& GetIntensity() { return intensity; }
  virtual glm::vec4& GetColor() { return color; }
};