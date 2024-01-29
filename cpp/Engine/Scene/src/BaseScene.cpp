#include "../include/BaseScene.h"

#include <Engine/Camera/include/BaseCamera.h>
#include <Engine/Light/include/BaseLight.h>
#include <Engine/Light/include/LightChannel.h>
#include <Engine/Model/include/BaseMaterial.h>
#include <Engine/Scene/include/SceneObject.h>

#include <ranges>

void DestroyObjects(SceneObject* root) {
  for (SceneObject* son : root->GetSons()) {
    DestroyObjects(son);
    son->DestroyImmediately();
  }
}

void BaseScene::OnCreate() {
  BaseObject::OnCreate();
  rootObject =
      Create<SceneObject>(rootObject, "RootObject", GetRoot(), GetFile());
  JsonUtils::ParseSceneObjectTree(graphics, rootObject, GetRoot(), GetFile(),
                                  this);
  JsonUtils::ParseSceneLightChannels(GetRoot(), GetFile(), this);
}

void BaseScene::OnStart() {
  BaseObject::OnStart();
  puts("--- Scene Model ---");
  rootObject->PrintSons();
  puts("--- Model Loaded ---");
}

void BaseScene::OnDestroy() {
  BaseObject::OnDestroy();

  DestroyObjects(rootObject);
  rootObject->DestroyImmediately();

  for (BaseMaterial* material : materials | std::views::values) {
    material->DestroyImmediately();
  }
}

BaseMaterial* BaseScene::GetMaterialByPath(const std::string& path) {
  if (path == "Unset") {
    throw std::runtime_error("please set material for model or mesh!");
  }
  if (materials.contains(path) == false) {
    materials[path] = Create<BaseMaterial>(false, GetRoot(), path);
  }
  return materials[path];
}
BaseCamera* BaseScene::GetCameraByName(const std::string& name) {
  return cameras[name];
}
BaseLight* BaseScene::GetLightByName(const std::string& name) {
  return lights[name];
}
LightChannel* BaseScene::GetLightChannelByName(const std::string& name) {
  return lightChannels[name];
}
void BaseScene::RegisterCamera(const std::string& name, BaseCamera* camera) {
  cameras[name] = camera;
}
void BaseScene::RegisterLight(const std::string& name, BaseLight* light) {
  lights[name] = light;
}
void BaseScene::RegisterLightChannel(const std::string& name,
                                LightChannel* channel) {
  lightChannels[name] = channel;
}