#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>

class SceneObject;

class BaseTransform {
  std::weak_ptr<SceneObject> _owner;

 public:
  void RegisterOwner(std::weak_ptr<SceneObject> other) { _owner = other; }

  // glm::vec3 relativeLeft = glm::vec3(1, 0, 0);
  // glm::vec3 absoluteLeft = glm::vec3(1, 0, 0);

  // glm::vec3 relativeUp = glm::vec3(0, 1, 0);
  // glm::vec3 absoluteUp = glm::vec3(0, 1, 0);

  // glm::vec3 relativeForward = glm::vec3(0, 0, 1);
  // glm::vec3 absoluteForward = glm::vec3(0, 0, 1);

  // glm::vec3 relativePosition = glm::vec3(0);
  // glm::vec3 absolutePosition = glm::vec3(0);

  // glm::vec3 relativeRotation = glm::vec3(0);
  // glm::vec3 absoluteRotation = glm::vec3(0);

  // glm::vec3 relativeScale = glm::vec3(0);
  // glm::vec3 absoluteScale = glm::vec3(0);

  // glm::mat4x4x4 absoluteTransform = glm::mat4x4x4(1);
  // glm::mat4x4x4 relativeTransform = glm::mat4x4x4(1);

  // glm::quat absoluteRotationQuat;
  // glm::quat relativeRotationQuat;

  BaseTransform()
      : relativePosition(0.0f),
        relativeRotation(0.0f),
        relativeScale(1.0f),
        relativeTransform(1.0f),
        absolutePosition(0.0f),
        absoluteRotation(0.0f),
        absoluteScale(1.0f),
        absoluteTransform(1.0f),
        relativeLeft(0.0f),
        relativeUp(0.0f),
        relativeForward(0.0f),
        absoluteLeft(0.0f),
        absoluteUp(0.0f),
        absoluteForward(0.0f),
        _owner(std::shared_ptr<SceneObject>(nullptr)) {}

  glm::vec3 ExtractTranslation(const glm::mat4x4& transform) {
    return glm::vec3(transform[3]);
  }

  glm::vec3 ExtractRotation(const glm::mat4x4& transform) {
    glm::quat q = glm::quat_cast(transform);
    return glm::eulerAngles(q);
  }

  glm::vec3 ExtractScale(const glm::mat4x4& transform) {
    glm::vec3 scale;
    scale.x = glm::length(glm::vec3(transform[0]));
    scale.y = glm::length(glm::vec3(transform[1]));
    scale.z = glm::length(glm::vec3(transform[2]));
    return scale;
  }

  void UpdateAbsoluteTransform();
  void UpdateRelativeTransform();

  const glm::mat4x4& GetRelativeTransform() const { return relativeTransform; }
  const glm::mat4x4& GetAbsoluteTransform() const { return absoluteTransform; }
  const glm::vec3& GetRelativePosition() const { return relativePosition; }
  const glm::vec3& GetAbsolutePosition() const { return absolutePosition; }
  const glm::vec3& GetRelativeRotation() const { return relativeRotation; }
  const glm::vec3& GetAbsoluteRotation() const { return absoluteRotation; }
  const glm::vec3& GetRelativeScale() const { return relativeScale; }
  const glm::vec3& GetAbsoluteScale() const { return absoluteScale; }

  const glm::vec3& GetRelativeForward() {
    relativeForward = glm::normalize(glm::vec3(-relativeTransform[2]));
    return relativeForward;
  }
  const glm::vec3& GetRelativeLeft() {
    relativeLeft = glm::normalize(glm::vec3(-relativeTransform[0]));
    return relativeLeft;
  }
  const glm::vec3& GetRelativeUp() {
    relativeUp = glm::normalize(glm::vec3(relativeTransform[1]));
    return relativeUp;
  }
  const glm::vec3& GetAbsoluteForward() {
    absoluteForward = glm::normalize(glm::vec3(-absoluteTransform[2]));
    return absoluteForward;
  }
  const glm::vec3& GetAbsoluteLeft() {
    absoluteLeft = glm::normalize(glm::vec3(-absoluteTransform[0]));
    return absoluteLeft;
  }
  const glm::vec3& GetAbsoluteUp() {
    absoluteUp = glm::normalize(glm::vec3(absoluteTransform[1]));
    return absoluteUp;
  }

  void SetRelativePosition(const glm::vec3& pos) {
    relativePosition = pos;
    UpdateAbsoluteTransform();
  }
  void SetAbsolutePosition(const glm::vec3& pos) {
    absolutePosition = pos;
    UpdateRelativeTransform();
  }
  void SetRelativeRotation(const glm::vec3& rot) {
    relativeRotation = rot;
    UpdateAbsoluteTransform();
  }
  void SetAbsoluteRotation(const glm::vec3& rot) {
    absoluteRotation = rot;
    UpdateRelativeTransform();
  }
  void SetRelativeScale(const glm::vec3& scale) {
    relativeScale = scale;
    UpdateAbsoluteTransform();
  }
  void SetAbsoluteScale(const glm::vec3& scale) {
    absoluteScale = scale;
    UpdateRelativeTransform();
  }

 private:
  glm::vec3 relativePosition;
  glm::vec3 relativeRotation;
  glm::vec3 relativeScale;

  glm::vec3 absolutePosition;
  glm::vec3 absoluteRotation;
  glm::vec3 absoluteScale;

  glm::mat4x4 relativeTransform;
  glm::mat4x4 absoluteTransform;

  glm::vec3 relativeLeft;
  glm::vec3 relativeUp;
  glm::vec3 relativeForward;

  glm::vec3 absoluteLeft;
  glm::vec3 absoluteUp;
  glm::vec3 absoluteForward;
};
