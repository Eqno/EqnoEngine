#pragma once

#include <Engine/Scene/include/SceneObject.h>

class GraphicsInterface;

class BaseCamera : public SceneObject {
 protected:
  glm::mat4x4 ViewMatrix = glm::mat4x4(1);
  glm::mat4x4 ProjMatrix = glm::mat4x4(1);

  float fovy = 45;
  float aspect = -1;
  float near = 0.1;
  float far = 1000;

  float sensitivityX = 1;
  float sensitivityY = 1;

  float moveSpeed = 0;
  float speedIncreasingRate = 0;

  float rotateX = 0;
  float rotateY = 0;
  GraphicsInterface* graphics;

 public:
  template <typename... Args>
  explicit BaseCamera(GraphicsInterface* graphics, Args&&... args)
      : graphics(graphics), SceneObject(std::forward<Args>(args)...) {}
  ~BaseCamera() override = default;

  void ParseAspect(const std::string& content) {
    if (content == "Auto") return;
    float width = stof(content.substr(0, content.find(':')));
    float height = stof(content.substr(content.find(':') + 1));
    aspect = width / height;
  }

  virtual void OnCreate() override;
  virtual void InitRotation(const glm::vec3& rot) {
    rotateX = rot.y;
    rotateY = rot.x;
  }
  virtual void OnUpdate() override;

  void UpdateViewMatrix();
  void UpdateProjMatrix();

  glm::mat4x4& GetViewMatrix();
  glm::mat4x4& GetProjMatrix();

  void PerformRotation();
  void PerformTraslation();
};