#include "../include/BaseTransform.h"

#include "Engine/Scene/include/SceneObject.h"

glm::mat4x4 BaseTransform::getAbsoluteTransform() {
  return glm::mat4x4(glm::vec4(absoluteRight, 0), glm::vec4(absoluteUp, 0),
                     glm::vec4(absoluteForward, 0), glm::vec4(0, 0, 0, 1));
}
