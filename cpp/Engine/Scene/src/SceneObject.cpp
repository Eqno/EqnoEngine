#include "../include/SceneObject.h"

#include <Engine/Scene/include/BaseScene.h>
#include <Engine/Utility/include/MathUtils.h>

#define VecInSubSpace(member)                                              \
  transform.absolute##member =                                             \
      parent->GetTransform().absoluteLeft * transform.relative##member.x + \
      parent->GetTransform().absoluteUp * transform.relative##member.y +   \
      parent->GetTransform().absoluteForward * transform.relative##member.z;

#define VecInGlobSpace(member) \
  transform.absolute##member = transform.relative##member;

void SceneObject::UpdateAbsoluteTransform() {
  if (parent != nullptr) {
    VecInSubSpace(Left);
    VecInSubSpace(Up);
    VecInSubSpace(Forward);
    VecInSubSpace(Position);
  } else {
    VecInGlobSpace(Left);
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
  transform.relativeLeft = MathUtils::rotate(
      glm::vec3(glm::length(transform.relativeLeft), 0, 0), radians(rot));
  transform.relativeUp = MathUtils::rotate(
      glm::vec3(0, glm::length(transform.relativeUp), 0), radians(rot));
  transform.relativeForward = MathUtils::rotate(
      glm::vec3(0, 0, glm::length(transform.relativeForward)), radians(rot));
  UpdateAbsoluteTransform();
}
void SceneObject::SetRelativeScale(const glm::vec3& sca) {
  transform.relativeLeft = glm::normalize(transform.relativeLeft) * sca.x;
  transform.relativeUp = glm::normalize(transform.relativeUp) * sca.y;
  transform.relativeForward = glm::normalize(transform.relativeForward) * sca.z;
  UpdateAbsoluteTransform();
}

void SceneObject::PrintSons() {
  std::cout << "Parent: " << _name << ", Sons: ";
  for (SceneObject* son : sons) {
    std::cout << son->_name << ", ";
  }
  std::cout << std::endl;
  for (SceneObject* son : sons) {
    son->PrintSons();
  }
}

glm::mat4x4& SceneObject::GetAbsoluteTransform() {
  transform.absoluteTransform = glm::mat4x4(
      glm::vec4(transform.absoluteLeft, 0), glm::vec4(transform.absoluteUp, 0),
      glm::vec4(transform.absoluteForward, 0),
      glm::vec4(transform.absolutePosition, 1));
  return transform.absoluteTransform;
}

glm::vec3& SceneObject::GetAbsolutePosition() {
  return transform.absolutePosition;
}

glm::vec3& SceneObject::GetAbsoluteForward() {
  return transform.absoluteForward;
}

void SceneObject::OnCreate() {
  BaseObject::OnCreate();
  scene = dynamic_cast<BaseScene*>(_owner);
  if (scene == nullptr) {
    throw std::runtime_error("please create camera through scene method!");
  }
  transform.RegisterOwner(this);
}