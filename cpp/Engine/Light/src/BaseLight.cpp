#include <Engine/Light/include/BaseLight.h>
#include <Engine/Scene/include/BaseScene.h>

void BaseLight::OnCreate() {
  SceneObject::OnCreate();
  scene->RegisterLight(name, this);

  intensity = JSON_CONFIG(Float, "Intensity");
  color = ParseGLMVec4(JSON_CONFIG(String, "Color"));
}