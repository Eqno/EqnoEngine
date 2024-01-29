#pragma once

#include <Engine/Light/include/BaseLight.h>

class SpotLight : public BaseLight {
 public:
  template <typename... Args>
  explicit SpotLight(Args&&... args) : BaseLight(std::forward<Args>(args)...) {}
  ~SpotLight() override = default;
};