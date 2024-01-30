#include <Engine/Light/include/BaseLight.h>
#include <Engine/Scene/include/BaseScene.h>

void BaseLight::OnCreate() {
  SceneObject::OnCreate();
  scene->RegisterLight(name, this);

  params.type = LightType::Unset;
  params.intensity = JSON_CONFIG(Float, "Intensity");
  params.color = ParseGLMVec4(JSON_CONFIG(String, "Color"));

  params.pos = GetAbsolutePosition();
  params.normal = GetAbsoluteForward();
}