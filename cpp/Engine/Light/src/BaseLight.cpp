#include <Engine/Light/include/BaseLight.h>
#include <Engine/Scene/include/BaseScene.h>

void BaseLight::OnCreate() {
  SceneObject::OnCreate();

  if (auto scenePtr = scene.lock()) {
    id = scenePtr->RegisterLight(
        name, std::dynamic_pointer_cast<BaseLight>(shared_from_this()));
  }

  intensity = JSON_CONFIG(Float, "Intensity");
  color = ParseGLMVec4(JSON_CONFIG(String, "Color"));
}

void BaseLight::OnDestroy() {
  SceneObject::OnDestroy();

  if (auto scenePtr = scene.lock()) {
    scenePtr->UnregisterLight(name);
  }
}