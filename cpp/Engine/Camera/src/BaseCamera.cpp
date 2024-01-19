#include <Engine/Camera/include/BaseCamera.h>
#include <Engine/System/include/BaseInput.h>
#include <Engine/System/include/GraphicsInterface.h>

std::unordered_map<std::string, BaseCamera*> BaseCamera::CameraMap;

glm::mat4x4 BaseCamera::GetViewMatrix() {
  return lookAt(transform.absolutePosition,
                transform.absolutePosition + transform.absoluteForward,
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

void BaseCamera::OnUpdate() {
  SceneObject::OnUpdate();
  PerformRotation();
  PerformTraslation();
}

void BaseCamera::PerformTraslation() {
  if (Input::Key::w) {
    SetRelativePosition(transform.relativePosition +
                        transform.relativeForward * moveSpeed);
  }
  if (Input::Key::a) {
    SetRelativePosition(transform.relativePosition +
                        transform.relativeLeft * moveSpeed);
  }
  if (Input::Key::s) {
    SetRelativePosition(transform.relativePosition -
                        transform.relativeForward * moveSpeed);
  }
  if (Input::Key::d) {
    SetRelativePosition(transform.relativePosition -
                        transform.relativeLeft * moveSpeed);
  }
  if (Input::Key::q) {
    SetRelativePosition(transform.relativePosition -
                        transform.relativeUp * moveSpeed);
  }
  if (Input::Key::e) {
    SetRelativePosition(transform.relativePosition +
                        transform.relativeUp * moveSpeed);
  }
}

void BaseCamera::PerformRotation() {
  static float mouseLastPosX = 0;
  static float mouseLastPosY = 0;

  float mouseDeltaX = Input::Mouse::posX - mouseLastPosX;
  float mouseDeltaY = Input::Mouse::posY - mouseLastPosY;

  mouseLastPosX = Input::Mouse::posX;
  mouseLastPosY = Input::Mouse::posY;

  if (Input::Mouse::left || Input::Mouse::right) {
    rotateY += sensitivityY * mouseDeltaY;
    rotateX -= sensitivityX * mouseDeltaX;
    SetRelativeRotation(glm::vec3(rotateY, rotateX, 0));
  }
}