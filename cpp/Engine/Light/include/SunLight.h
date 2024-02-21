#pragma once

#include <Engine/Light/include/BaseLight.h>
#include <vulkan/vulkan_core.h>

class SunLight : public BaseLight {
 public:
  template <typename... Args>
  explicit SunLight(Args&&... args) : BaseLight(std::forward<Args>(args)...) {}
  ~SunLight() override = default;

  virtual void OnCreate() override {
    BaseLight::OnCreate();
    type = LightType::Sun;
  }

  virtual void UpdateViewMatrix() override {
    viewMatrix = lookAt(transform.absolutePosition,
                        transform.absolutePosition + transform.absoluteForward,
                        transform.absoluteUp);
  }

  virtual void UpdateProjMatrix() override {
    projMatrix = glm::ortho(-200, 200, -200, 200);
    projMatrix[1][1] *= -1;
  }
};