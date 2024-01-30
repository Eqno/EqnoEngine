#pragma once

#include <Engine/Scene/include/SceneObject.h>

class BaseLight : public SceneObject {
 protected:
  LightData params;

 public:
  template <typename... Args>
  explicit BaseLight(Args&&... args)
      : SceneObject(std::forward<Args>(args)...) {}
  ~BaseLight() override = default;

  virtual void OnCreate() override;
  virtual LightData& GetParams() { return params; }
};