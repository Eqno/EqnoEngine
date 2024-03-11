#pragma once

#include <Engine/Scene/include/SceneObject.h>

#include <mutex>

class BaseLight : public SceneObject {
 protected:
  int id = -1;
  LightType type = LightType::Unset;
  float intensity = 1;
  glm::vec4 color = Vec4One;

  std::mutex updateMatrixMutex;
  glm::mat4 viewMatrix = Mat4x4One;
  glm::mat4 projMatrix = Mat4x4One;

 public:
  template <typename... Args>
  explicit BaseLight(Args&&... args)
      : SceneObject(std::forward<Args>(args)...) {}
  ~BaseLight() override = default;

  virtual void OnCreate() override;
  virtual void OnDestroy() override;
  virtual void OnUpdate() override {
    if (updateMatrixMutex.try_lock()) {
      UpdateViewMatrix();
      UpdateProjMatrix();
      updateMatrixMutex.unlock();
    }
  }

  virtual int GetId() { return id; }
  virtual LightType& GetType() { return type; }
  virtual float& GetIntensity() { return intensity; }
  virtual glm::vec4& GetColor() { return color; }

  virtual glm::vec3& GetPosition() { return GetAbsolutePosition(); }
  virtual glm::vec3& GetNormal() { return GetAbsoluteForward(); }

  virtual void UpdateViewMatrix() = 0;
  virtual void UpdateProjMatrix() = 0;

  virtual std::mutex& GetMatrixLock() { return updateMatrixMutex; }
  virtual glm::mat4& GetViewMatrix() { return viewMatrix; }
  virtual glm::mat4& GetProjMatrix() { return projMatrix; }
};