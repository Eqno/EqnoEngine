#include <Engine/Light/include/LightChannel.h>
#include <Engine/Scene/include/BaseScene.h>

void LightChannel::OnCreate() {
  BaseObject::OnCreate();

  if (auto ownerPtr = GetOwner().lock()) {
    scene = std::static_pointer_cast<BaseScene>(ownerPtr);
    if (auto scenePtr = scene.lock()) {
      scenePtr->RegisterLightChannel(
          name, std::static_pointer_cast<LightChannel>(shared_from_this()));
    } else {
      throw std::runtime_error(
          "please create light channel through scene method!");
    }
  }
}
void LightChannel::OnDestroy() {
  BaseObject::OnDestroy();

  if (auto scenePtr = scene.lock()) {
    scenePtr->UnregisterLightChannel(name);
  }
}