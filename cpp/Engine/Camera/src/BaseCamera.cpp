#include <Engine/Camera/include/BaseCamera.h>
#include <Engine/System/include/GraphicsInterface.h>

std::unordered_map<std::string, BaseCamera*> BaseCamera::CameraMap;

glm::mat4x4 BaseCamera::GetViewMatrix() {
  return lookAt(transform.absolutePosition, transform.absoluteForward,
                transform.absoluteUp);
}
glm::mat4x4 BaseCamera::GetProjMatrix() {
  if (aspect < 0) {
    return glm::perspective(glm::radians(fovy), graphics->GetViewportAspect(),
                            near, far);
  } else {
    return glm::perspective(glm::radians(fovy), aspect, near, far);
  }
}