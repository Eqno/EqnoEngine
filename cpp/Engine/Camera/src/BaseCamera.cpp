#include <Engine/Camera/include/BaseCamera.h>
#include <Engine/System/include/BaseInput.h>
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

void BaseCamera::OnUpdate() {
  SceneObject::OnUpdate();

  if (Input::Key::wDown) {
    puts("W down!");
  }
  if (Input::Key::aDown) {
    puts("A down!");
  }
  if (Input::Key::sDown) {
    puts("S down!");
  }
  if (Input::Key::dDown) {
    puts("D down!");
  }
  if (Input::Key::w) {
    puts("W press!");
  }
  if (Input::Key::a) {
    puts("A press!");
  }
  if (Input::Key::s) {
    puts("S press!");
  }
  if (Input::Key::d) {
    puts("D press!");
  }
}