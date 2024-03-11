#pragma once

#include <Engine/Scene/include/SceneObject.h>

#include <mutex>

class GraphicsInterface;

class BaseCamera : public SceneObject {
 protected:
  std::weak_ptr<GraphicsInterface> graphics;

  std::mutex updateMatrixMutex;
  glm::mat4 viewMatrix = Mat4x4One;
  glm::mat4 projMatrix = Mat4x4One;

  float fovy = 45;
  float aspect = -1;
  float near = 0.1f;
  float far = 1000;

  float maxFov = 80;
  float minFov = 5;

  float sensitivityX = 1;
  float sensitivityY = 1;
  float sensitivityZ = 1;

  float moveSpeed = 0;
  float speedIncreasingRate = 0;

  float maxMoveSpeed = 0.01;
  float minMoveSpeed = 1000;

  float rotateX = 0;
  float rotateY = 0;

 public:
  template <typename... Args>
  explicit BaseCamera(std::weak_ptr<GraphicsInterface> graphics, Args&&... args)
      : graphics(graphics), SceneObject(std::forward<Args>(args)...) {}
  ~BaseCamera() override = default;

  static float ParseAspect(const std::string& content) {
    if (content == "Auto") return -1;
    float width = stof(content.substr(0, content.find(':')));
    float height = stof(content.substr(content.find(':') + 1));
    return width / height;
  }

  virtual void OnCreate() override;
  virtual void InitRotation(const glm::vec3& rot) {
    rotateX = rot.y;
    rotateY = rot.x;
  }
  virtual void OnUpdate() override;
  virtual void OnDestroy() override;

  virtual void PerformRotation();
  virtual void PerformTraslation();

  virtual void UpdateViewMatrix();
  virtual void UpdateProjMatrix();

  virtual std::mutex& GetMatrixLock() { return updateMatrixMutex; }
  glm::mat4& GetViewMatrix() { return viewMatrix; }
  glm::mat4& GetProjMatrix() { return projMatrix; }
};