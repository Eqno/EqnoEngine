#pragma once

#include <Engine/Light/include/BaseLight.h>

class SunLight : public BaseLight {
 public:
  template <typename... Args>
  explicit SunLight(Args&&... args) : BaseLight(std::forward<Args>(args)...) {}
  ~SunLight() override = default;

  virtual void OnCreate() override {
    BaseLight::OnCreate();
    type = LightType::Sun;
  }
};