#pragma once

#include <Engine/Scene/include/SceneObject.h>

class BaseCamera : public SceneObject {
 protected:
 public:
  template <typename... Args>
  explicit BaseCamera(GraphicsInterface* graphics, Args&&... args)
      : graphics(graphics), SceneObject(std::forward<Args>(args)...) {}
  ~BaseCamera() override = default;
};