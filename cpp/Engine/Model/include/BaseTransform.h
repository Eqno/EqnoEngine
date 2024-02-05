#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <memory>

class SceneObject;

class BaseTransform {
  std::weak_ptr<SceneObject> _owner;

 public:
  void RegisterOwner(std::weak_ptr<SceneObject> other) { _owner = other; }

  glm::vec3 relativeLeft = glm::vec3(1, 0, 0);
  glm::vec3 absoluteLeft = glm::vec3(1, 0, 0);

  glm::vec3 relativeUp = glm::vec3(0, 1, 0);
  glm::vec3 absoluteUp = glm::vec3(0, 1, 0);

  glm::vec3 relativeForward = glm::vec3(0, 0, 1);
  glm::vec3 absoluteForward = glm::vec3(0, 0, 1);

  glm::vec3 relativePosition = glm::vec3(0);
  glm::vec3 absolutePosition = glm::vec3(0);

  glm::mat4x4 absoluteTransform = glm::mat4x4(1);
};
