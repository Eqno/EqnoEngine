#include <Engine/Camera/include/BaseCamera.h>
#include <Engine/Scene/include/BaseScene.h>
#include <Engine/Scene/include/SceneObject.h>
#include <Engine/System/include/BaseInput.h>
#include <Engine/System/include/GraphicsInterface.h>

void BaseCamera::UpdateViewMatrix() {
  viewMatrix =
      lookAt(GetAbsolutePosition(),
             GetAbsolutePosition() + GetAbsoluteForward(), GetAbsoluteUp());
}

void BaseCamera::UpdateProjMatrix() {
  if (aspect < 0) {
    if (auto graphicsPtr = graphics.lock()) {
      projMatrix = glm::perspective(
          glm::radians(fovy), graphicsPtr->GetViewportAspect(), near, far);
    }
  } else {
    projMatrix = glm::perspective(glm::radians(fovy), aspect, near, far);
  }
  projMatrix[1][1] *= -1;
}

void BaseCamera::OnCreate() {
  SceneObject::OnCreate();

  if (auto scenePtr = scene.lock()) {
    scenePtr->RegisterCamera(
        name, std::static_pointer_cast<BaseCamera>(shared_from_this()));
  }

  aspect = ParseAspect(JSON_CONFIG(String, "Aspect"));
  fovy = JSON_CONFIG(Float, "FOVy");
  near = JSON_CONFIG(Float, "Near");
  far = JSON_CONFIG(Float, "Far");

  maxFov = JSON_CONFIG(Float, "MaxFov");
  minFov = JSON_CONFIG(Float, "MinFov");

  sensitivityX = JSON_CONFIG(Float, "SensitivityX");
  sensitivityY = JSON_CONFIG(Float, "SensitivityY");
  sensitivityZ = JSON_CONFIG(Float, "SensitivityZ");

  moveSpeed = JSON_CONFIG(Float, "MoveSpeed");
  speedIncreasingRate = JSON_CONFIG(Float, "speedIncreasingRate");

  maxMoveSpeed = JSON_CONFIG(Float, "MaxMoveSpeed");
  minMoveSpeed = JSON_CONFIG(Float, "MinMoveSpeed");
}

void BaseCamera::OnUpdate() {
  SceneObject::OnUpdate();

  PerformRotation();
  PerformTraslation();

  if (updateMatrixMutex.try_lock()) {
    UpdateViewMatrix();
    UpdateProjMatrix();
    updateMatrixMutex.unlock();
  }
}

void BaseCamera::OnDestroy() {
  SceneObject::OnDestroy();

  if (auto scenePtr = scene.lock()) {
    scenePtr->UnregisterCamera(name);
  }
}

void BaseCamera::PerformTraslation() {
  if (Input::Key::w) {
    SetRelativePosition(GetRelativePosition() +
                        GetRelativeForward() * moveSpeed * GameDeltaTime);
  }
  if (Input::Key::a) {
    SetRelativePosition(GetRelativePosition() +
                        GetRelativeLeft() * moveSpeed * GameDeltaTime);
  }
  if (Input::Key::s) {
    SetRelativePosition(GetRelativePosition() -
                        GetRelativeForward() * moveSpeed * GameDeltaTime);
  }
  if (Input::Key::d) {
    SetRelativePosition(GetRelativePosition() -
                        GetRelativeLeft() * moveSpeed * GameDeltaTime);
  }
  if (Input::Key::q) {
    SetRelativePosition(GetRelativePosition() -
                        GetRelativeUp() * moveSpeed * GameDeltaTime);
  }
  if (Input::Key::e) {
    SetRelativePosition(GetRelativePosition() +
                        GetRelativeUp() * moveSpeed * GameDeltaTime);
  }
  if (Input::Mouse::scrollY > 0) {
    if (Input::Key::leftShift) {
      moveSpeed = std::min(maxMoveSpeed, moveSpeed * 2);
    } else {
      fovy = std::max(minFov, fovy - sensitivityZ);
    }
  }
  if (Input::Mouse::scrollY < 0) {
    if (Input::Key::leftShift) {
      moveSpeed = std::max(minMoveSpeed, moveSpeed / 2);
    } else {
      fovy = std::min(maxFov, fovy + sensitivityZ);
    }
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