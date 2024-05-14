#include "../include/SceneObject.h"

#include <Engine/Scene/include/BaseScene.h>
#include <Engine/Utility/include/MathUtils.h>

//#define VecInSubSpace(member, origin)                                         \
//  transform.absolute##member =                                                \
//      origin +                                                                \
//      parentPtr->GetTransform().absoluteLeft * transform.relative##member.x + \
//      parentPtr->GetTransform().absoluteUp * transform.relative##member.y +   \
//      parentPtr->GetTransform().absoluteForward *                             \
//          transform.relative##member.z;
//
// #define VecInGlobSpace(member) \
//  transform.absolute##member = transform.relative##member;
//
// void SceneObject::UpdateAbsoluteTransform() {
//  if (auto parentPtr = parent.lock()) {
//    VecInSubSpace(Left, Vec3Zero);
//    VecInSubSpace(Up, Vec3Zero);
//    VecInSubSpace(Forward, Vec3Zero);
//    VecInSubSpace(Position, parentPtr->GetTransform().absolutePosition);
//  } else {
//    VecInGlobSpace(Left);
//    VecInGlobSpace(Up);
//    VecInGlobSpace(Forward);
//    VecInGlobSpace(Position);
//  }
//  for (std::shared_ptr<SceneObject> son : GetSons()) {
//    son->UpdateAbsoluteTransform();
//  }
//}
//
// void SceneObject::UpdateRelativeTransform() {
//  if (auto parentPtr = parent.lock()) {
//    glm::mat4 parentAbsTransformInv = glm::inverse(
//        glm::mat4(glm::vec4(parentPtr->GetTransform().absoluteLeft, 0),
//                  glm::vec4(parentPtr->GetTransform().absoluteUp, 0),
//                  glm::vec4(parentPtr->GetTransform().absoluteForward, 0),
//                  glm::vec4(parentPtr->GetTransform().absolutePosition, 1)));
//
//    transform.relativeLeft =
//        glm::vec3(parentAbsTransformInv * glm::vec4(transform.absoluteLeft,
//        1));
//    transform.relativeUp =
//        glm::vec3(parentAbsTransformInv * glm::vec4(transform.absoluteUp, 1));
//    transform.relativeForward = glm::vec3(
//        parentAbsTransformInv * glm::vec4(transform.absoluteForward, 1));
//    transform.relativePosition = glm::vec3(
//        parentAbsTransformInv * glm::vec4(transform.absolutePosition, 1));
//  } else {
//    transform.relativeLeft = transform.absoluteLeft;
//    transform.relativeUp = transform.absoluteUp;
//    transform.relativeForward = transform.absoluteForward;
//    transform.relativePosition = transform.absolutePosition;
//  }
//}
//
// const glm::mat4x4& SceneObject::GetAbsoluteTransform() const {
//  transform.absoluteTransform = glm::mat4x4(
//      glm::vec4(transform.absoluteLeft, 0), glm::vec4(transform.absoluteUp,
//      0), glm::vec4(transform.absoluteForward, 0),
//      glm::vec4(transform.absolutePosition, 1));
//  return transform.absoluteTransform;
//}
//
// const glm::vec3& SceneObject::GetAbsolutePosition() const {
//  return transform.absolutePosition;
//}
//
// const glm::vec3& SceneObject::GetAbsoluteForward() const {
//  return transform.absoluteForward;
//}
//
// const glm::vec3& SceneObject::GetRelativePosition() const {
//  return transform.relativePosition;
//}
// const glm::vec3& SceneObject::GetRelativeForward() const {
//  return transform.relativeForward;
//}
//
// void SceneObject::SetRelativePosition(const glm::vec3& pos) {
//  transform.relativePosition = pos;
//  UpdateAbsoluteTransform();
//}
// void SceneObject::SetRelativeRotation(const glm::vec3& rot) {
//  transform.relativeLeft = MathUtils::rotate(
//      glm::vec3(glm::length(transform.relativeLeft), 0, 0), radians(rot));
//  transform.relativeUp = MathUtils::rotate(
//      glm::vec3(0, glm::length(transform.relativeUp), 0), radians(rot));
//  transform.relativeForward = MathUtils::rotate(
//      glm::vec3(0, 0, glm::length(transform.relativeForward)), radians(rot));
//  UpdateAbsoluteTransform();
//}
// void SceneObject::SetRelativeScale(const glm::vec3& sca) {
//  transform.relativeLeft = glm::normalize(transform.relativeLeft) * sca.x;
//  transform.relativeUp = glm::normalize(transform.relativeUp) * sca.y;
//  transform.relativeForward = glm::normalize(transform.relativeForward) *
//  sca.z; UpdateAbsoluteTransform();
//}
//
// void SceneObject::SetAbsolutePosition(const glm::vec3& pos) {
//  transform.absolutePosition = pos;
//  UpdateRelativeTransform();
//}
// void SceneObject::SetAbsoluteRotation(const glm::vec3& rot) {
//  glm::mat4 absRotation = MathUtils::RotationMatrixFromVector(rot);
//  transform.absoluteLeft = glm::vec3(absRotation[0]);
//  transform.absoluteUp = glm::vec3(absRotation[1]);
//  transform.absoluteForward = glm::vec3(absRotation[2]);
//  UpdateRelativeTransform();
//}
// void SceneObject::SetAbsoluteScale(const glm::vec3& sca) {
//  transform.absoluteLeft = glm::normalize(transform.absoluteLeft) * sca.x;
//  transform.absoluteUp = glm::normalize(transform.absoluteUp) * sca.y;
//  transform.absoluteForward = glm::normalize(transform.absoluteForward) *
//  sca.z; UpdateRelativeTransform();
//}

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

void SceneObject::OnCreate() {
  BaseObject::OnCreate();
  if (auto parentPtr = parent.lock()) {
    parentPtr->AddToSons(static_pointer_cast<SceneObject>(shared_from_this()));
  }
  if (auto ownerPtr = GetOwner().lock()) {
    scene = std::static_pointer_cast<BaseScene>(ownerPtr);
    if (!scene.lock()) {
      PRINT_AND_THROW_ERROR("please create camera through scene method!");
    }
  }
  transform.RegisterOwner(
      std::static_pointer_cast<SceneObject>(shared_from_this()));

  JsonUtils::UseDefaultTransform(
      static_pointer_cast<SceneObject>(shared_from_this()),
      GetRoot() + GetFile());
}

void SceneObject::OnDestroy() {
  BaseObject::OnDestroy();

  for (std::shared_ptr<SceneObject> son : GetSons()) {
    son->Destroy();
  }
  if (auto parentPtr = parent.lock()) {
    parentPtr->RemoveFromSons(
        static_pointer_cast<SceneObject>(shared_from_this()));
  }
}