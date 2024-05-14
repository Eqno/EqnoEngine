#pragma once

#include <Engine/Light/include/BaseLight.h>
#include <Engine/System/include/GraphicsInterface.h>

class SpotLight : public BaseLight {
  std::weak_ptr<GraphicsInterface> graphics;

  float fovy = 45;
  float aspect = -1;

 public:
  template <typename... Args>
  explicit SpotLight(std::weak_ptr<GraphicsInterface> graphics, Args&&... args)
      : graphics(graphics), BaseLight(std::forward<Args>(args)...) {}
  ~SpotLight() override = default;

  virtual void OnCreate() override {
    BaseLight::OnCreate();
    type = LightType::Spot;

    aspect = BaseCamera::ParseAspect(JSON_CONFIG(String, "Aspect"));
    fovy = JSON_CONFIG(Float, "FOVy");
    near = JSON_CONFIG(Float, "Near");
    far = JSON_CONFIG(Float, "Far");
  }

  virtual void UpdateViewMatrix() override {
    viewMatrix =
        lookAt(GetAbsolutePosition(),
               GetAbsolutePosition() + GetAbsoluteForward(), GetAbsoluteUp());
  }

  virtual void UpdateProjMatrix() override {
    if (aspect < 0) {
      if (auto graphicsPtr = graphics.lock()) {
        projMatrix = glm::perspective(
            glm::radians(fovy), graphicsPtr->GetViewportAspect(), near, far);
      }
    } else {
      projMatrix = glm::perspective(glm::radians(fovy), aspect, near, far);
    }
    projMatrix[1][1] *= -1;
  }
#pragma region Params
  float GetFovy() const { return fovy; }
  void SetFovy(float value) { fovy = value; }

  float GetAspect() const { return aspect; }
  void SetAspect(float value) { aspect = value; }
#pragma endregion
};