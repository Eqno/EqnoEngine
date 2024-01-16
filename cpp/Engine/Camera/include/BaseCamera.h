#pragma once

#include <Engine/Scene/include/SceneObject.h>

class GraphicsInterface;

class BaseCamera : public SceneObject {
 protected:
  float fovy = 45;
  float aspect = -1;
  float near = 0.1;
  float far = 1000;
  GraphicsInterface* graphics;

 public:
  static std::unordered_map<std::string, BaseCamera*> CameraMap;

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

  virtual void OnCreate() override {
    SceneObject::OnCreate();
    CameraMap[name] = this;

    ParseAspect(JSON_CONFIG(String, "Aspect"));
    fovy = JSON_CONFIG(Float, "FOVy");
    near = JSON_CONFIG(Float, "Near");
    far = JSON_CONFIG(Float, "Far");
  }
  virtual void OnUpdate() override;

  glm::mat4x4 GetViewMatrix();
  glm::mat4x4 GetProjMatrix();
};