#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>

class SceneObject;

class BaseTransform {
  SceneObject* _owner = nullptr;

 public:
  void RegisterOwner(SceneObject* other) { _owner = other; }

  glm::vec3 relativeRight = glm::vec3(1, 0, 0);
  glm::vec3 absoluteRight = glm::vec3(1, 0, 0);

  glm::vec3 relativeUp = glm::vec3(0, 1, 0);
  glm::vec3 absoluteUp = glm::vec3(0, 1, 0);

  glm::vec3 relativeForward = glm::vec3(0, 0, 1);
  glm::vec3 absoluteForward = glm::vec3(0, 0, 1);

  glm::vec3 relativePosition = glm::vec3(0);
  glm::vec3 absolutePosition = glm::vec3(0);
};
