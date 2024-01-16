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

void SceneObject::OnCreate() {
  BaseObject::OnCreate();
  transform.RegisterOwner(this);
}

void SceneObject::OnUpdate() {}