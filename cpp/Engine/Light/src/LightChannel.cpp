#include <Engine/Light/include/LightChannel.h>
#include <Engine/Scene/include/BaseScene.h>

void LightChannel::OnCreate() {
  BaseObject::OnCreate();

  scene = dynamic_cast<BaseScene*>(_owner);
  if (scene == nullptr) {
    throw std::runtime_error(
        "please create light channel through scene method!");
  }
  scene->RegisterLightChannel(name, this);
}