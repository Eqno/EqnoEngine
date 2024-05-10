#include <Engine/Model/include/BaseTransform.h>
#include <Engine/Scene/include/SceneObject.h>

void BaseTransform::UpdateAbsoluteTransform() {
  relativeTransform = glm::translate(glm::mat4x4(1.0f), relativePosition) *
                      glm::toMat4(glm::quat(relativeRotation)) *
                      glm::scale(glm::mat4x4(1.0f), relativeScale);
  auto object = static_pointer_cast<SceneObject>(_owner.lock());
  if (auto parent = object->GetParent().lock()) {
    absoluteTransform = parent->GetAbsoluteTransform() * relativeTransform;

    absolutePosition = ExtractTranslation(absoluteTransform);
    absoluteRotation = ExtractRotation(absoluteTransform);
    absoluteScale = ExtractScale(absoluteTransform);
  } else {
    absolutePosition = relativePosition;
    absoluteRotation = relativeRotation;
    absoluteScale = relativeScale;

    absoluteTransform = glm::translate(glm::mat4x4(1.0f), absolutePosition) *
                        glm::toMat4(glm::quat(absoluteRotation)) *
                        glm::scale(glm::mat4x4(1.0f), absoluteScale);
  }
  for (auto son : object->GetSons()) {
    son->UpdateAbsoluteTransform();
  }
}

void BaseTransform::UpdateRelativeTransform() {
  absoluteTransform = glm::translate(glm::mat4x4(1.0f), absolutePosition) *
                      glm::toMat4(glm::quat(absoluteRotation)) *
                      glm::scale(glm::mat4x4(1.0f), absoluteScale);
  auto object = static_pointer_cast<SceneObject>(_owner.lock());
  if (auto parent = object->GetParent().lock()) {
    glm::mat4x4 invParentTransform =
        glm::inverse(parent->GetAbsoluteTransform());
    glm::mat4x4 relativeTransform = invParentTransform * absoluteTransform;

    relativePosition = ExtractTranslation(relativeTransform);
    relativeRotation = ExtractRotation(relativeTransform);
    relativeScale = ExtractScale(relativeTransform);
  } else {
    relativePosition = absolutePosition;
    relativeRotation = absoluteRotation;
    relativeScale = absoluteScale;

    relativeTransform = glm::translate(glm::mat4x4(1.0f), relativePosition) *
                        glm::toMat4(glm::quat(relativeRotation)) *
                        glm::scale(glm::mat4x4(1.0f), relativeScale);
  }
  for (auto son : object->GetSons()) {
    son->UpdateAbsoluteTransform();
  }
}