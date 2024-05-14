#pragma once

#include <Engine/Scene/include/SceneObject.h>

#include <mutex>

class BaseLight : public SceneObject {
 protected:
  int id = -1;
  LightType type = LightType::Unset;
  float intensity = 1;
  glm::vec4 color = Vec4One;

  float near = 0.1f;
  float far = 1000;

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

  virtual const glm::vec3& GetPosition() const { return GetAbsolutePosition(); }
  virtual const glm::vec3& GetNormal() { return GetAbsoluteForward(); }

  virtual void UpdateViewMatrix() = 0;
  virtual void UpdateProjMatrix() = 0;

  virtual std::mutex& GetMatrixLock() { return updateMatrixMutex; }
  virtual glm::mat4& GetViewMatrix() { return viewMatrix; }
  virtual glm::mat4& GetProjMatrix() { return projMatrix; }

#pragma region Params
  void SetIntensity(float value) { intensity = value; }
  void SetColor(const glm::vec4& value) { color = value; }

  float GetNear() const { return near; }
  void SetNear(float value) { near = value; }

  float GetFar() const { return far; }
  void SetFar(float value) { far = value; }
#pragma endregion
};