#pragma once

#include <Engine/Light/include/BaseLight.h>
#include <vulkan/vulkan_core.h>

class SunLight : public BaseLight {
  float left = -100;
  float right = 100;
  float bottom = -100;
  float top = 100;

 public:
  template <typename... Args>
  explicit SunLight(Args&&... args) : BaseLight(std::forward<Args>(args)...) {}
  ~SunLight() override = default;

  virtual void OnCreate() override {
    BaseLight::OnCreate();
    type = LightType::Sun;

    left = JSON_CONFIG(Float, "Left");
    right = JSON_CONFIG(Float, "Right");
    bottom = JSON_CONFIG(Float, "Bottom");
    top = JSON_CONFIG(Float, "Top");
    near = JSON_CONFIG(Float, "Near");
    far = JSON_CONFIG(Float, "Far");
  }

  virtual void OnUpdate() { BaseLight::OnUpdate(); }

  virtual void UpdateViewMatrix() override {
    viewMatrix =
        lookAt(GetAbsolutePosition(),
               GetAbsolutePosition() + GetAbsoluteForward(), GetAbsoluteUp());
  }

  virtual void UpdateProjMatrix() override {
    projMatrix = glm::ortho(left, right, bottom, top, near, far);
    projMatrix[1][1] *= -1;
  }

#pragma region Params
  float GetLeft() const { return left; }
  void SetLeft(float value) { left = value; }

  float GetRight() const { return right; }
  void SetRight(float value) { right = value; }

  float GetBottom() const { return bottom; }
  void SetBottom(float value) { bottom = value; }

  float GetTop() const { return top; }
  void SetTop(float value) { top = value; }
#pragma endregion
};