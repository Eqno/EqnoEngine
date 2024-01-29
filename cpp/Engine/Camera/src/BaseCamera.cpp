#include <Engine/Camera/include/BaseCamera.h>
#include <Engine/Scene/include/BaseScene.h>
#include <Engine/Scene/include/SceneObject.h>
#include <Engine/System/include/BaseInput.h>
#include <Engine/System/include/GraphicsInterface.h>

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

void BaseCamera::OnCreate() {
  SceneObject::OnCreate();
  scene->AddCamera(name, this);

  ParseAspect(JSON_CONFIG(String, "Aspect"));
  fovy = JSON_CONFIG(Float, "FOVy");
  near = JSON_CONFIG(Float, "Near");
  far = JSON_CONFIG(Float, "Far");

  sensitivityX = JSON_CONFIG(Float, "SensitivityX");
  sensitivityY = JSON_CONFIG(Float, "SensitivityY");

  moveSpeed = JSON_CONFIG(Float, "MoveSpeed");
  speedIncreasingRate = JSON_CONFIG(Float, "speedIncreasingRate");
}

void BaseCamera::OnUpdate() {
  // std::cout << DeltaTime << std::endl;

  SceneObject::OnUpdate();
  PerformRotation();
  PerformTraslation();
}

void BaseCamera::PerformTraslation() {
  if (Input::Key::w) {
    SetRelativePosition(transform.relativePosition +
                        transform.relativeForward * moveSpeed * DeltaTime);
  }
  if (Input::Key::a) {
    SetRelativePosition(transform.relativePosition +
                        transform.relativeLeft * moveSpeed * DeltaTime);
  }
  if (Input::Key::s) {
    SetRelativePosition(transform.relativePosition -
                        transform.relativeForward * moveSpeed * DeltaTime);
  }
  if (Input::Key::d) {
    SetRelativePosition(transform.relativePosition -
                        transform.relativeLeft * moveSpeed * DeltaTime);
  }
  if (Input::Key::q) {
    SetRelativePosition(transform.relativePosition -
                        transform.relativeUp * moveSpeed * DeltaTime);
  }
  if (Input::Key::e) {
    SetRelativePosition(transform.relativePosition +
                        transform.relativeUp * moveSpeed * DeltaTime);
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
    rotateY += sensitivityY * mouseDeltaY * DeltaTime;
    rotateX -= sensitivityX * mouseDeltaX * DeltaTime;
    SetRelativeRotation(glm::vec3(rotateY, rotateX, 0));
  }
}