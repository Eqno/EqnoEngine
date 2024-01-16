#include "../include/SceneObject.h"

#include <Engine/Utility/include/MathUtils.h>

#define VecInSubSpace(member)                                               \
  transform.absolute##member =                                              \
      parent->GetTransform().absoluteRight * transform.relative##member.x + \
      parent->GetTransform().absoluteUp * transform.relative##member.y +    \
      parent->GetTransform().absoluteForward * transform.relative##member.z;

#define VecInGlobSpace(member) \
  transform.absolute##member = transform.relative##member;

void SceneObject::UpdateAbsoluteTransform() {
  if (parent != nullptr) {
    VecInSubSpace(Right);
    VecInSubSpace(Up);
    VecInSubSpace(Forward);
    VecInSubSpace(Position);
  } else {
    VecInGlobSpace(Right);
    VecInGlobSpace(Up);
    VecInGlobSpace(Forward);
    VecInGlobSpace(Position);
  }
  for (SceneObject* son : sons) {
    son->UpdateAbsoluteTransform();
  }
}

void SceneObject::SetRelativePosition(const glm::vec3& pos) {
  transform.relativePosition = pos;
  UpdateAbsoluteTransform();
}
void SceneObject::SetRelativeRotation(const glm::vec3& rot) {
  transform.relativeRight = MathUtils::rotate(
      glm::vec3(glm::length(transform.relativeRight), 0, 0), rot);
  transform.relativeUp = MathUtils::rotate(
      glm::vec3(0, glm::length(transform.relativeUp), 0), rot);
  transform.relativeForward = MathUtils::rotate(
      glm::vec3(0, 0, glm::length(transform.relativeForward)), rot);
  UpdateAbsoluteTransform();
}
void SceneObject::SetRelativeScale(const glm::vec3& sca) {
  transform.relativeRight = glm::normalize(transform.relativeRight) * sca.x;
  transform.relativeUp = glm::normalize(transform.relativeUp) * sca.y;
  transform.relativeForward = glm::normalize(transform.relativeForward) * sca.z;
  UpdateAbsoluteTransform();
}

void SceneObject::AddRelativePosition(const glm::vec3& pos) {
  transform.relativePosition += pos;
  UpdateAbsoluteTransform();
}

void SceneObject::AddRelativeRotation(const glm::vec3& rot) {
  transform.relativeRight = MathUtils::rotate(transform.relativeRight, rot);
  transform.relativeUp = MathUtils::rotate(transform.relativeUp, rot);
  transform.relativeForward = MathUtils::rotate(transform.relativeForward, rot);
  UpdateAbsoluteTransform();
}

void SceneObject::AddRelativeScale(const glm::vec3& sca) {
  transform.relativeRight = glm::normalize(transform.relativeRight) *
                            (glm::length(transform.relativeRight) + sca.x);
  transform.relativeUp = glm::normalize(transform.relativeUp) *
                         (glm::length(transform.relativeUp) + sca.y);
  transform.relativeForward = glm::normalize(transform.relativeForward) *
                              (glm::length(transform.relativeForward) + sca.z);
  UpdateAbsoluteTransform();
}

glm::mat4x4 SceneObject::GetAbsoluteTransform() {
  return glm::mat4x4(glm::vec4(transform.absoluteRight, 0),
                     glm::vec4(transform.absoluteUp, 0),
                     glm::vec4(transform.absoluteForward, 0),
                     glm::vec4(transform.absolutePosition, 1));
}

void SceneObject::OnCreate() {
  BaseObject::OnCreate();
  transform.RegisterOwner(this);
}

void SceneObject::OnUpdate() {}