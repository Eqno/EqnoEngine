#include "../include/SceneObject.h"

#include <Engine/Scene/include/BaseScene.h>
#include <Engine/Utility/include/MathUtils.h>

#define VecInSubSpace(member, origin)                                         \
  transform.absolute##member =                                                \
      origin +                                                                \
      parentPtr->GetTransform().absoluteLeft * transform.relative##member.x + \
      parentPtr->GetTransform().absoluteUp * transform.relative##member.y +   \
      parentPtr->GetTransform().absoluteForward *                             \
          transform.relative##member.z;

#define VecInGlobSpace(member) \
  transform.absolute##member = transform.relative##member;

void SceneObject::UpdateAbsoluteTransform() {
  if (auto parentPtr = parent.lock()) {
    VecInSubSpace(Left, Vec3Zero);
    VecInSubSpace(Up, Vec3Zero);
    VecInSubSpace(Forward, Vec3Zero);
    VecInSubSpace(Position, parentPtr->GetTransform().absolutePosition);
  } else {
    VecInGlobSpace(Left);
    VecInGlobSpace(Up);
    VecInGlobSpace(Forward);
    VecInGlobSpace(Position);
  }
  for (std::shared_ptr<SceneObject> son : GetSons()) {
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
  for (std::shared_ptr<SceneObject> son : sons) {
    std::cout << son->_name << ", ";
  }
  std::cout << std::endl;
  for (std::shared_ptr<SceneObject> son : sons) {
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
  if (auto parentPtr = parent.lock()) {
    parentPtr->AddToSons(dynamic_pointer_cast<SceneObject>(shared_from_this()));
  }
  if (auto ownerPtr = GetOwner().lock()) {
    scene = std::dynamic_pointer_cast<BaseScene>(ownerPtr);
    if (!scene.lock()) {
      throw std::runtime_error("please create camera through scene method!");
    }
  }
  transform.RegisterOwner(
      std::dynamic_pointer_cast<SceneObject>(shared_from_this()));
}

void SceneObject::OnDestroy() {
  BaseObject::OnDestroy();

  for (std::shared_ptr<SceneObject> son : GetSons()) {
    son->Destroy();
  }
  if (auto parentPtr = parent.lock()) {
    auto iter = parentPtr->GetSons().begin();
    while (iter != parentPtr->GetSons().end()) {
      if (iter->get() == this) {
        parentPtr->GetSons().erase(iter);
        break;
      }
      iter++;
    }
  }
}