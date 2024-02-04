#include "../include/BaseScene.h"

#include <Engine/Camera/include/BaseCamera.h>
#include <Engine/Light/include/BaseLight.h>
#include <Engine/Light/include/LightChannel.h>
#include <Engine/Model/include/BaseMaterial.h>
#include <Engine/Scene/include/SceneObject.h>
#include <assimp/types.h>

#include <ranges>

void DestroyObjects(SceneObject* root) {
  for (SceneObject* son : root->GetSons()) {
    DestroyObjects(son);
  }
  root->DestroyImmediately();
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
  for (LightChannel* lightChannel : lightChannels | std::views::values) {
    lightChannel->DestroyImmediately();
  }
}

BaseMaterial* BaseScene::GetMaterialByPath(const std::string& path,
                                           aiMaterial* matData) {
  std::string name = path + ": " + matData->GetName().C_Str();
  if (materials.contains(name) == false) {
    materials[name] = Create<BaseMaterial>(matData, false, GetRoot(), path);
  }
  return materials[name];
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